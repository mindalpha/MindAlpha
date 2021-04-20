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

def parse_s3_url(s3_url):
    from urllib.parse import urlparse
    r = urlparse(s3_url, allow_fragments=False)
    if r.scheme not in ('s3', 's3a'):
        message = "invalid s3 url: %r" % (s3_url,)
        raise ValueError(message)
    path = r.path.lstrip('/')
    return r.netloc, path

def get_s3_client():
    import os
    import boto3
    endpoint = os.environ.get('AWS_ENDPOINT')
    if endpoint is None:
        s3 = boto3.client('s3')
    else:
        if not endpoint.startswith('http://') and not endpoint.startswith('https://'):
            endpoint = 'http://' + endpoint
        s3 = boto3.client('s3', endpoint_url=endpoint)
    return s3

def get_s3_dir_size(dir_path):
    bucket, path = parse_s3_url(dir_path)
    if not path.endswith('/'):
        path += '/'
    s3 = get_s3_client()
    objs = s3.list_objects(Bucket=bucket, Prefix=path)
    size = 0
    if 'Contents' in objs:
        for obj in objs['Contents']:
            size += obj['Size']
    return size

def s3_file_exists(file_path):
    bucket, path = parse_s3_url(file_path)
    s3 = get_s3_client()
    try:
        s3.head_object(Bucket=bucket, Key=path)
    except:
        return False
    else:
        return True
