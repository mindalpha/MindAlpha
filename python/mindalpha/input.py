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

def shuffle_df(df, num_workers):
    from pyspark.sql import functions as F
    df = df.withColumn('srand', F.rand())
    df = df.repartition(2 * num_workers, 'srand')
    print('shuffle df to partitions {}'.format(df.rdd.getNumPartitions()))
    df = df.sortWithinPartitions('srand')
    df = df.drop('srand')
    return df

def read_kudu(spark_session, url, column_name, sql=None, condition_select_conf='', shuffle=False, num_workers=1):
    from pyspark.sql import SQLContext
    from pyspark.sql import DataFrame

    queryCols=[]
    has_rand_column = False
    with open(column_name, 'r') as f_column_name:
        for line in f_column_name:
            line = line.split(" ")[-1].strip()
            if line == 'rand':
                has_rand_column = True
            queryCols.append(line)
    if not has_rand_column:
        print('append rand column by default')
        queryCols.append('rand')
    print('total cols from kudu: {}'.format(len(queryCols)))

    sc = spark_session.sparkContext
    ssqlContext = SQLContext(sc)._ssql_ctx
    jsparkSession = spark_session._jsparkSession
    if condition_select_conf == '':
        queryKudu = sc._jvm.com.mobvista.dataflow.apis.kuduUtils.QueryKudu.readKudu(jsparkSession, url, queryCols)
    else:
        print('use condition select conf: {}'.format(condition_select_conf))
        queryKudu = sc._jvm.com.mobvista.dataflow.apis.kuduUtils.QueryKudu.readKudu(jsparkSession, url, queryCols, condition_select_conf)
    kudu_df_tmp = DataFrame(queryKudu, ssqlContext)
    kudu_df_tmp.createOrReplaceTempView("kudu_df_tmp")
    if sql is not None:
        kudu_df = spark_session.sql(sql)
    else:
        kudu_df = spark_session.sql('select * from kudu_df_tmp')

    if shuffle and num_workers > 1:
        kudu_df = shuffle_df(kudu_df, num_workers)
    else:
        print("ignore shuffle")
    if not has_rand_column:
        kudu_df = kudu_df.drop('rand')
    return kudu_df

def read_s3_csv(spark_session, url, shuffle=False, num_workers=1,
                header=False, nullable=False, delimiter="\002", encoding="UTF-8", schema_str=None):
    from .url_utils import use_s3a
    reader = (spark_session
             .read
             .format('csv')
             .option("header", str(bool(header)).lower())
             .option("nullable", str(bool(nullable)).lower())
             .option("delimiter", delimiter)
             .option("encoding", encoding))

    if schema_str is not None:
        reader = reader.schema(schema_str)

    df = reader.load(use_s3a(url))
    print("debug @ Schema {}".format(df.printSchema()))
    if shuffle and num_workers > 1:
        df = shuffle_df(df, num_workers)
    else:
        print("ignore shuffle")
    return df

def read_s3_image(spark_session, url):
    from .url_utils import use_s3a
    df = spark_session.read.format('image').option('dropInvalid', 'true').load(use_s3a(url))
    return df
