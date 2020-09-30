import tensorflow as tf
import numpy as np
from PIL import Image

with open('mnist_200.pb', 'rb') as f:
    a = tf.parse_tensor(f.read(), tf.float32)

with tf.Session() as sess:
    b = a.eval(session=sess)

b = np.squeeze(b)*256
b = b.astype(np.uint8)

img = Image.fromarray(b, 'L')
img.show()


