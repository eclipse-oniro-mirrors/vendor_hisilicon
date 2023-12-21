#!/usr/bin/env python3

# Copyright (c) 2021 Huawei Device Co., Ltd.
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

import os
import sys
from PIL import Image

if len(sys.argv) < 2:
	print('Usage: {} gif outdir'.format(sys.argv[0]))
	exit(-1)

gifimg = sys.argv[1]
outdir = sys.argv[2]

im = Image.open(gifimg)
if not os.path.exists(outdir):
	os.mkdir(outdir)

i = 0
try:
	while True:
		im.seek(i)
		im.save(os.path.join(outdir, 'frame' + str(i) + '.png'))
		i += 1
except:
	pass
print('images:', i)
