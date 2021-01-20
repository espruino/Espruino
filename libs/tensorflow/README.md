Tensorflow for Espruino
=======================

Tensorflow code is Licensed under the Apache License. See `LICENSE` for more info.

Created with:

```Bash
# https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/experimental/micro
# Assuming Espruino is in /workspace/Espruino
cd ~/workspace
git clone https://github.com/tensorflow/tensorflow --depth=1
cd tensorflow
make -f tensorflow/lite/micro/tools/make/Makefile
make -f tensorflow/lite/micro/tools/make/Makefile TARGET=make TAGS="disco_f746ng" generate_projects
cp -R tensorflow/lite/micro/tools/make/gen/make_x86_64/prj/hello_world/make/* ~/workspace/Espruino/libs/tensorflow
# apply patches
cd ~/workspace/Espruino
cat libs/tensorflow/patches/* | patch -p1
```

## Actually using it

Follow the steps here: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/examples/hello_world/create_sine_model.ipynb

Then right at the end when you have your model, do:

```Python
import base64;
print("var model=atob(\""+base64.b64encode(tflite_model)+"\");")
```

To output to base64 that you can easily stick in your code.

And to use it, do something like:

```JS
var model=atob("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");


var tf = require("tensorflow").create(2048, model);
tf.getInput()[0] = x;
tf.invoke();
print(tf.getOutput()[0]);
```

Example code (based off the tutorial linked above) to do the training is:

```Python
import tensorflow as tf
# Numpy is a math library
import numpy as np
# Matplotlib is a graphing library
import matplotlib.pyplot as plt
# math is Python's math library
import math

# We'll generate this many sample datapoints
SAMPLES = 1000

# Set a "seed" value, so we get the same random numbers each time we run this
# notebook
np.random.seed(1337)

# Generate a uniformly distributed set of random numbers in the range from
# 0 to 2pi, which covers a complete sine wave oscillation
x_values = np.random.uniform(low=0, high=2*math.pi, size=SAMPLES)

# Shuffle the values to guarantee they're not in order
np.random.shuffle(x_values)

# Calculate the corresponding sine values
y_values = np.sin(x_values)

# Add a small random number to each y value
y_values += 0.1 * np.random.randn(*y_values.shape)

# We'll use 60% of our data for training and 20% for testing. The remaining 20%
# will be used for validation. Calculate the indices of each section.
TRAIN_SPLIT =  int(0.6 * SAMPLES)
TEST_SPLIT = int(0.2 * SAMPLES + TRAIN_SPLIT)

# Use np.split to chop our data into three parts.
# The second argument to np.split is an array of indices where the data will be
# split. We provide two indices, so the data will be divided into three chunks.
x_train, x_test, x_validate = np.split(x_values, [TRAIN_SPLIT, TEST_SPLIT])
y_train, y_test, y_validate = np.split(y_values, [TRAIN_SPLIT, TEST_SPLIT])

# Double check that our splits add up correctly
assert (x_train.size + x_validate.size + x_test.size) ==  SAMPLES

# We'll use Keras to create a simple model architecture
from tensorflow.keras import layers
model_2 = tf.keras.Sequential()

# First layer takes a scalar input and feeds it through 16 "neurons". The
# neurons decide whether to activate based on the 'relu' activation function.
model_2.add(layers.Dense(16, activation='relu', input_shape=(1,)))

# The new second layer may help the network learn more complex representations
model_2.add(layers.Dense(16, activation='relu'))

# Final layer is a single neuron, since we want to output a single value
model_2.add(layers.Dense(1))

# Compile the model using a standard optimizer and loss function for regression
model_2.compile(optimizer='rmsprop', loss='mse', metrics=['mae'])

history_2 = model_2.fit(x_train, y_train, epochs=600, batch_size=16,
                    validation_data=(x_validate, y_validate))

# Calculate and print the loss on our test dataset
loss = model_2.evaluate(x_test, y_test)

# Make predictions based on our test dataset
predictions = model_2.predict(x_test)

# Graph the predictions against the actual values
plt.clf()
plt.title('Comparison of predictions and actual values')
plt.plot(x_test, y_test, 'b.', label='Actual')
plt.plot(x_test, predictions, 'r.', label='Predicted')
plt.legend()
plt.show()

# Convert the model to the TensorFlow Lite format without quantization
converter = tf.lite.TFLiteConverter.from_keras_model(model_2)
tflite_model = converter.convert()

# Save the model to disk
open("sine_model.tflite", "wb").write(tflite_model)

# Convert the model to the TensorFlow Lite format with quantization
converter = tf.lite.TFLiteConverter.from_keras_model(model_2)
converter.optimizations = [tf.lite.Optimize.OPTIMIZE_FOR_SIZE]
tflite_model = converter.convert()

# Save the model to disk
open("sine_model_quantized.tflite", "wb").write(tflite_model)

import base64;
print("var model=atob(\""+base64.b64encode(tflite_model)+"\");")
```

