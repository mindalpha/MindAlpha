#
# Copyright 2021 Mobvista
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import json
import time
from typing import Dict, Any

from . import patching_pickle
import os
import requests
import consul
from consul.base import Timeout
import glob
import pickle
import shutil



class Experiment(object):
    def __init__(self,
                 job_name,
                 experiment_name,
                 business_name,
                 owner,
                 schedule_interval,
                 func,
                 start_date,
                 end_date=None,
                 upstream_job_names=None,
                 extra_dag_conf: Dict[str, Any] = {},
                 enable_auth_token=True,
                 is_local_test=False,
                 delay_backfill=False,
                 airflow_host=None
                 ):
        self.job_name = job_name
        self.experiment_name = experiment_name
        self.business_name = business_name
        self.owner = owner
        self.schedule_interval = schedule_interval
        self.func = func
        self.start_date = start_date
        self.end_date = end_date
        self.upstream_job_names = upstream_job_names
        self.extra_dag_conf = extra_dag_conf
        self.enable_auth_token = enable_auth_token
        self.is_local_test = is_local_test
        self.delay_backfill = delay_backfill
        self.airflow_host = airflow_host

    def submit_backfill(self):
        experiment_operator = ExperimentOperate(self.enable_auth_token, self.is_local_test, self.airflow_host)
        experiment_operator.check_airflow_hosts(experiment_operator.airflow_host)

        job_obj = Job(name=self.job_name,
                      experiment=self.experiment_name,
                      business=self.business_name,
                      owner=self.owner,
                      schedule_interval=self.schedule_interval,
                      func=self.func,
                      start_date=self.start_date,
                      end_date=self.end_date,
                      upstream_job_names=self.upstream_job_names,
                      submit_type=Job._BACKFILL,
                      extra_dag_conf=self.extra_dag_conf,
                      local_pickle_tmp_dir=experiment_operator.local_pickle_tmp_dir,
                      )
        experiment_operator.check_exist_dag_conf(job_obj)
        ExperimentOperate.start_job(experiment_operator, job_obj, self.delay_backfill)

    def submit_online(self):
        experiment_operator = ExperimentOperate(self.enable_auth_token, self.is_local_test, self.airflow_host)
        experiment_operator.check_airflow_hosts(experiment_operator.airflow_host)

        job_obj = Job(name=self.job_name,
                      experiment=self.experiment_name,
                      business=self.business_name,
                      owner=self.owner,
                      schedule_interval=self.schedule_interval,
                      func=self.func,
                      start_date=self.start_date,
                      end_date=None,
                      upstream_job_names=self.upstream_job_names,
                      submit_type=Job._ONLINE,
                      extra_dag_conf=self.extra_dag_conf,
                      local_pickle_tmp_dir=experiment_operator.local_pickle_tmp_dir
                      )
        experiment_operator.check_exist_dag_conf(job_obj)
        ExperimentOperate.start_job(experiment_operator, job_obj)

class ExperimentOperate(object):
    _SUCCESS = 'SUCCESS'
    _FAILED = 'FAILED'
    _LOCAL_AIRFLOW_HOST = 'http://localhost:8080'
    _LOCAL_AIRFLOW_PICKLE_TMP_DIR = '/opt/airflow/dags/s3-sync'
    _LOCAL_JUPYTER_PICKLE_TMP_DIR = '.experiment_sdk/pickle_dir'
    _AIRFLOW_HOST_ENV_KEY = 'AIRFLOW_HOST'
    _CONSUL_HOST_ENV_KEY = 'CONSUL_HOST'
    _AIRFLOW_S3_SYNC_PATH_ENV_KEY = 'AIRFLOW_S3_SYNC_PATH'
    _AIRFLOW_REST_AUTHORIZATION_TOKEN = 'AIRFLOW_REST_AUTHORIZATION_TOKEN'

    def __init__(self, enable_auth_token, is_local_test, customer_airflow_host):

        self.enable_auth_token = enable_auth_token
        self.is_local_test = is_local_test
        self.airflow_host = self.get_airflow_hosts(customer_airflow_host)  # from env
        self.local_pickle_tmp_dir = self.get_local_pickle_tmp_dir()
        self.airflow_s3_sync_path = self.get_airflow_s3_sync_path()  # from env
        self.authorization_token = self.get_airflow_rest_authorization_token()

    def print_airflow_web_hosts(self):
        print(f"airflow web url: {self.airflow_host}")

    def sync_from_s3_to_local(self, s3_path, local_path):
        try:
            shutil.rmtree(local_path)
            os.makedirs(local_path)
            result = os.system(f"aws s3 sync {s3_path} {local_path}")
        except Exception as err:
            raise RuntimeError(f"s3 sync error: {err}")
        return result

    def check_exist_dag_conf(self, job_obj):
        # pass
        if not self.is_local_test:
            if job_obj.submit_type == job_obj._ONLINE:
                s3_dir = '/'.join([self.airflow_s3_sync_path, job_obj.business, Job._SUBMIT_DIR[Job._ONLINE]])
                local_dir = '/'.join(
                    [self.local_pickle_tmp_dir, job_obj.business, Job._SUBMIT_DIR[Job._ONLINE]])
            elif job_obj.submit_type == job_obj._BACKFILL:
                s3_dir = '/'.join(
                    [self.airflow_s3_sync_path, job_obj.business, Job._SUBMIT_DIR[Job._BACKFILL],
                     job_obj.experiment])
                local_dir = '/'.join(
                    [self.local_pickle_tmp_dir, job_obj.business, Job._SUBMIT_DIR[Job._BACKFILL],
                     job_obj.experiment])
            else:
                raise ValueError(f"no such submit_type: {job_obj.submit_type}")

            result = self.sync_from_s3_to_local(s3_dir, local_dir)
            if result != 0:
                raise RuntimeError("check exist dag conf error")
            tmp_local_dir = local_dir
        else:
            if job_obj.submit_type == job_obj._ONLINE:
                tmp_local_dir = '/'.join(
                    [self.local_pickle_tmp_dir, job_obj.business, Job._SUBMIT_DIR[Job._ONLINE]])
            else:
                tmp_local_dir = '/'.join(
                    [self.local_pickle_tmp_dir, job_obj.business, Job._SUBMIT_DIR[Job._BACKFILL],
                     job_obj.experiment])

        if os.path.isdir(tmp_local_dir):
            files = glob.glob(tmp_local_dir + '/**/*.pickle', recursive=True)
            if len(files) != 0:
                random_file = files[0]
                f = open(random_file, 'rb')
                random_job_obj = pickle.load(f)
                random_dag_conf = random_job_obj.dag_conf
                for key in random_dag_conf.keys():
                    r_v = random_dag_conf[key]
                    j_v = job_obj.dag_conf[key]
                    if r_v != j_v:
                        raise ValueError(
                            f"unexpect dag conf: {key}:{j_v}. it must be same with exist dag {job_obj.dag_id}: {key}:{r_v}")

    def start_job(self, job_obj, delay_backfill=False):
        print("start job")
        if not self.is_local_test:
            if ExperimentOperate.dump_pickle(job_obj) == ExperimentOperate._SUCCESS:
                self.upload_file_to_s3(job_obj)
            else:
                raise RuntimeError("dump pickle error")
        else:
            ExperimentOperate.dump_pickle(job_obj)
        # if not delay_backfill:
            # self.unpause_dag(job_obj)

    def dump_pickle(job_obj):
        local_pickle_file_path = job_obj.local_pickle_file_path
        if os.path.exists(local_pickle_file_path):
            os.remove(local_pickle_file_path)
        file_dir = os.path.split(local_pickle_file_path)[0]
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)
        with open(local_pickle_file_path, 'wb') as f:
            try:
                patching_pickle.dump(job_obj, f)
                return ExperimentOperate._SUCCESS
            except:
                return ExperimentOperate._FAILED
            finally:
                f.close()

    def get_consul_host(customer_consul_host):
        if not customer_consul_host:
            consul_host = os.getenv(ExperimentOperate._CONSUL_HOST_ENV_KEY, '')
        else:
            consul_host = customer_consul_host
        if not consul_host:
            raise ValueError(f"CONSUL_HOST:{consul_host} not set")
        try:
            tmp_list = consul_host.split(':')
            host = tmp_list[0]
            port = tmp_list[1].replace('/', '')
        except Exception as err:
            raise ValueError(f"consul_host parse err: {consul_host}, should be like '127.0.0.1:8500' . err:{err}")
        return host, port

    def get_airflow_hosts(self, customer_airflow_host):
        if self.is_local_test:
            airflow_host = ExperimentOperate._LOCAL_AIRFLOW_HOST
        else:
            if not customer_airflow_host:
                airflow_host = os.getenv(ExperimentOperate._AIRFLOW_HOST_ENV_KEY, '')
            else:
                airflow_host = customer_airflow_host
        return airflow_host

    def check_airflow_hosts(self, airflow_host):
        if not airflow_host:
            raise ValueError(f"AIRFLOW_HOST:{airflow_host} not set")

    def get_airflow_s3_sync_path(self):
        try:
            airflow_s3_sync_path = os.getenv(ExperimentOperate._AIRFLOW_S3_SYNC_PATH_ENV_KEY)
        except Exception as err:
            raise AttributeError(f"AIRFLOW_S3_SYNC_PATH not set in env; err: {err}")
        return airflow_s3_sync_path

    def get_local_pickle_tmp_dir(self):
        if self.is_local_test:
            local_pickle_tmp_dir = ExperimentOperate._LOCAL_AIRFLOW_PICKLE_TMP_DIR
        else:
            local_pickle_tmp_dir = ExperimentOperate._LOCAL_JUPYTER_PICKLE_TMP_DIR
        return local_pickle_tmp_dir

    def upload_file_to_s3(self, job_obj):
        local_pickle_file_path = job_obj.local_pickle_file_path
        if local_pickle_file_path.startswith(self.local_pickle_tmp_dir):
            suffix = local_pickle_file_path.replace(self.local_pickle_tmp_dir, '')
            s3_path = self.airflow_s3_sync_path + suffix
            try:
                os.system("aws s3 cp {0} {1}".format(local_pickle_file_path, s3_path))
                print('success upload pickle to s3: ' + local_pickle_file_path, s3_path)
            except Exception as err:
                raise RuntimeError(f"s3 err: {err}")

    def get_airflow_rest_authorization_token(self):
        airflow_rest_authorization_token = os.getenv(ExperimentOperate._AIRFLOW_REST_AUTHORIZATION_TOKEN, '')
        print(f"airflow_rest_authorization_token: {airflow_rest_authorization_token}")
        if not airflow_rest_authorization_token:
            raise RuntimeError(f"airflow_rest_authorization_token env not set.")
        return airflow_rest_authorization_token

    def get_airflow_rest_authorization_token_jwt(self, user, passwd):
        authorization_token = ''
        if self.enable_auth_token:
            endpoit = f'{self.airflow_host}/api/v1/security/login'
            headers = {'Content-Type': 'application/json'}
            data = {"username": user, "password": passwd, "refresh": "true", "provider": "db"}
            response = requests.post(url=endpoit, headers=headers, data=json.dumps(data))
            response_dict = json.loads(response.text)
            access_token = response_dict['access_token']
            authorization_token = 'Bearer ' + access_token
        return authorization_token

    def request_post(self, url):
        # headers = {'Content-Type': 'application/x-www-form-urlencoded'}
        if self.enable_auth_token:
            headers = {'rest_api_plugin_http_token': self.authorization_token}
        else:
            headers = {'Content-Type': 'application/x-www-form-urlencoded'}
        response = requests.post(url, headers=headers)
        return response

    def request_post_jwt(self, url):
        # headers = {'Content-Type': 'application/x-www-form-urlencoded'}
        if self.is_local_test:
            headers = {'Content-Type': 'application/x-www-form-urlencoded'}
        else:
            headers = {'Authorization': self.authorization_token, 'Content-Type': 'application/x-www-form-urlencoded'}
        response = requests.post(url, headers=headers)
        return response

    def set_airflow_pools(self, pool_name, slot_count):
        description = f'test'
        url = f'{self.airflow_host}/admin/rest_api/api?api=pool&cmd=set&pool_name={pool_name}&slot_count={slot_count}&pool_description={description}'
        self.request_post(url)

    def list_dag(self):
        url = f'{self.airflow_host}/admin/rest_api/api?api=list_dags'
        response = self.request_post(url)
        text = json.loads(response.text)
        dags_info = text['output']['stdout']
        return dags_info

    def unpause(self, dag_id):
        url = f'{self.airflow_host}/admin/rest_api/api?api=unpause&dag_id={dag_id}'
        response = self.request_post(url)
        text = json.loads(response.text)
        result = text['output']['stdout']
        return result

    def unpause_dag(self, job_obj):
        dag_id = job_obj.dag_id
        is_unpause = False
        for i in range(5):
            time.sleep(6)
            dags_info = self.list_dag()
            if dag_id in dags_info:
                result = self.unpause(dag_id)
                print(result)
                if result:
                    print(f'success start DAG : {dag_id}')
                    is_unpause = True
                    break
        if not is_unpause:
            raise ValueError(f'DAG : {dag_id} does not exist. Please open airflow web and check again.')


class Job(object):
    _BACKFILL = 'backfill'
    _ONLINE = 'online'
    _SUBMIT_DIR = {_BACKFILL: 'backfill_dir', _ONLINE: 'online_dir'}

    def __init__(self,
                 name,
                 experiment,
                 business,
                 schedule_interval,
                 func,
                 submit_type,
                 start_date,
                 end_date,
                 owner,
                 upstream_job_names,
                 extra_dag_conf,  # type: Optional[Dict[str, Any]]
                 local_pickle_tmp_dir
                 ):
        self.name = name
        self.experiment = experiment
        self.business = business
        self.func = func
        self.schedule_interval = schedule_interval
        self.submit_type = submit_type
        self.catchup = self.get_catchup(submit_type)
        self.start_date = start_date
        self.end_date = end_date
        self.owner = self.get_owner(owner)
        self.upstream_job_names = self.get_upstream_job_names(upstream_job_names)
        self.extra_dag_conf: Dict[str, Any] = extra_dag_conf
        self.pickle_file_name = self.name + '.pickle'
        self.dag_id = self.get_dag_id(submit_type)
        self.local_pickle_tmp_dir = local_pickle_tmp_dir
        self.local_pickle_file_path = self._get_local_pickle_path(submit_type)
        self.dag_conf = self.generate_dag_conf()

    def _print_dict(self):
        print(self.__dict__)

    def get_upstream_job_names(self, upstream_job_names):
        if upstream_job_names:
            if type(upstream_job_names) != list:
                raise ValueError(
                    f"upstream_job_names should be a list. example: upstream_job_names = ['business_experiment_job1','business_experiment_job2']")
            return upstream_job_names
        else:
            return upstream_job_names

    def generate_dag_conf(self):
        dag_conf = {}
        dag_conf['dag_id'] = self.dag_id
        dag_conf['schedule_interval'] = self.schedule_interval
        dag_conf['owner'] = self.owner
        dag_conf['start_date'] = self.start_date
        dag_conf['end_date'] = self.end_date
        dag_conf['catchup'] = self.catchup
        dag_conf.update(self.extra_dag_conf)
        return dag_conf

    def get_dag_id(self, submit_type):
        if submit_type == self._BACKFILL:
            dag_id = f'{self.business}_{self.experiment}_{Job._BACKFILL}'
        else:
            dag_id = f'{self.business}_{Job._ONLINE}'
        return dag_id

    def get_catchup(self, submit_type):
        return True if submit_type == self._BACKFILL else False

    def get_owner(self, owner):
        return owner if owner else self.business

    def _get_local_pickle_path(self, submit_type):
        store_dir_path = '/'.join(
            [self.local_pickle_tmp_dir, self.business, Job._SUBMIT_DIR[submit_type], self.experiment])
        if not os.path.isdir(store_dir_path):
            os.makedirs(store_dir_path)
        pickle_file_path = '/'.join([store_dir_path, self.pickle_file_name])
        return pickle_file_path
