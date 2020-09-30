import tensorflow as tf
import numpy as np
from PIL import Image
from tensorflow.python.framework import graph_util
from tensorflow.python.framework import graph_io

with open('2020_01_22_19_08_26_4975_1_4.pb', 'rb') as f:
    a = tf.parse_tensor(f.read(), tf.float32)

with tf.Session() as sess:
    b = a.eval(session=sess)

b = np.squeeze(b)*256
b = b.astype(np.uint8)

img = Image.fromarray(b, 'L')
img.show()


