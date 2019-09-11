Tensorflow for Espruino
=======================


#### https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/experimental/micro
git clone https://github.com/tensorflow/tensorflow --depth=1
mv tensorflow git-tensorflow
cd git-tensorflow
make -f tensorflow/lite/experimental/micro/tools/make/Makefile
make -f tensorflow/lite/experimental/micro/tools/make/Makefile TARGET=make TAGS="disco_f746ng" generate_projects
cp -R tensorflow/lite/experimental/micro/tools/make/gen/make_x86_64/prj/hello_world/* ..
cd ..
mv tensorflow/tensorflow/lite/experimental/micro/examples/hello_world/* .
