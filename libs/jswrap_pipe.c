#include "jswrap_pipe.h"

/*JSON{ "type":"library",
        "class" : "Pipe",
        "description" : ["This is the Pipe container for async related IO." ]
}*/

JsVar* PipeGetArray(const char *name, bool create) {
  JsVar *arrayName = jsvFindChildFromString(execInfo.root, name, create);
  JsVar *arr = jsvSkipName(arrayName);
  if (!arr && create) {
    arr = jsvNewWithFlags(JSV_ARRAY);
    jsvSetValueOfName(arrayName, arr);
  }
  jsvUnLock(arrayName);
  return arr;
}

static bool _pipe(JsVar* source, JsVar* destination, JsVar* chunkSize, JsVar* position) {
  JsVarInt Bytes_Read = 0;
  if(source && destination && chunkSize && position) {
    JsVar * Buffer = jsvNewFromEmptyString();
    JsVarInt Position = jsvGetInteger(position);
    if(Buffer) {// do we have enough memory?
      JsVar *ReadFunc = jspGetNamedField(source, "read");
      JsVar *WriteFunc = jspGetNamedField(destination, "write");
      if (jsvIsFunction(ReadFunc) && jsvIsFunction(WriteFunc)) { // do the objects have the necessary methods on them?
        JsVar *ReadArgs[3];
        ReadArgs[0] = Buffer;
        ReadArgs[1] = chunkSize;
        ReadArgs[2] = position;
        JsVar *BytesRead = jspExecuteFunction(ReadFunc, source, 3, ReadArgs);
        Bytes_Read = jsvGetInteger(BytesRead);
        if(Bytes_Read > 0)
        {
          JsVar *WriteArgs[3];
          WriteArgs[0] = Buffer;
          WriteArgs[1] = BytesRead;
          WriteArgs[2] = position;
          jsvUnLock(jspExecuteFunction(WriteFunc, destination, 3, WriteArgs));
          jsvSetInteger(position, (Position+Bytes_Read));
        }
        jsvUnLock(BytesRead);
      } else {
        if(!jsvIsFunction(ReadFunc)) {
          jsError("Source Stream does not implement the required read(buffer, length, position) method.");
        }
        if(!jsvIsFunction(WriteFunc)) {
          jsError("Destination Stream does not implement the required write(buffer, length, position) method.");
        }
      }
      jsvUnLock(ReadFunc);
      jsvUnLock(WriteFunc);
      jsvUnLock(Buffer);
    }
  }
  return Bytes_Read > 0;
}

/*JSON{ "type":"idle", "generate" : "jswrap_pipe_idle" }*/
bool jswrap_pipe_idle() {
  bool ret = false;
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes",false);
  if (arr) {
    JsvArrayIterator it;
    jsvArrayIteratorNew(&it, arr);
    while (jsvArrayIteratorHasElement(&it)) {
      JsVar *pipe = jsvArrayIteratorGetElement(&it);
      JsVar *position = jsvObjectGetChild(pipe,"Position",0);
      JsVar *chunkSize = jsvObjectGetChild(pipe,"ChunkSize",0);
      JsVar *source = jsvObjectGetChild(pipe,"Source",0);
      JsVar *destination = jsvObjectGetChild(pipe,"Destination",0);
      if(!_pipe(source, destination, chunkSize, position)) { // when no more chunks are possible, execute the callback.
        jsiQueueObjectCallbacks(pipe, "#oncomplete", &pipe, 1);
        JsVar *idx = jsvArrayIteratorGetIndex(&it);
        jsvRemoveChild(arr,idx);
        jsvUnLock(idx);
      } else {
        ret = true;
      }
      jsvUnLock(source);
      jsvUnLock(destination);
      jsvUnLock(pipe);
      jsvUnLock(chunkSize);
      jsvUnLock(position);
      jsvArrayIteratorNext(&it);
    }
    jsvArrayIteratorFree(&it);
    jsvUnLock(arr);
  }
  return ret;
}

/*JSON{ "type":"kill", "generate" : "jswrap_pipe_kill" }*/
void jswrap_pipe_kill() {
  // now remove all pipes...
  {
    JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", false);
    if (arr) {
      jsvRemoveAllChildren(arr);
      jsvUnLock(arr);
    }
  }
}

/*JSON{  "type" : "staticmethod", "class" : "fs", "name" : "pipe",
         "generate" : "jswrap_pipe",
         "params" : [ ["source", "JsVar", "The source file/stream that will send content."],
                      ["destination", "JsVar", "The destination file/stream that will receive content from the source."],
                      ["chunkSize", "JsVar", "The amount of data to pipe from source to destination at a time."],
                      ["callback", "JsVar", "a function to call when the pipe activity is complete."] ]
}*/
//fs.pipe(source,destination,4,onCompleteHandler);
void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* ChunkSize, JsVar* callback) {
  JsVar *arr = PipeGetArray(JS_HIDDEN_CHAR_STR"OpenPipes", true);
  if (arr) {// out of memory?
    if (source && dest && ChunkSize) {
      JsVar *ReadFunc = jspGetNamedField(source, "read");
      JsVar *WriteFunc = jspGetNamedField(dest, "write");
      if(jsvIsFunction(ReadFunc)) {
        if(jsvIsFunction(WriteFunc)) {
          JsVar *pipe = jspNewObject(0, "Pipe");
          if(pipe) {// out of memory?
            if(callback) {
              jsvAddNamedChild(pipe, callback, "#oncomplete");
            }
            jsvArrayPush(arr, pipe);
            JsVar* Position = jsvNewFromInteger(0);
            if(Position) {
              jsvUnLock(jsvAddNamedChild(pipe, Position, "Position"));
              jsvUnLock(jsvAddNamedChild(pipe, ChunkSize, "ChunkSize"));
              jsvUnLock(jsvAddNamedChild(pipe, source, "Source"));
              jsvUnLock(jsvAddNamedChild(pipe, dest, "Destination"));
            }
            jsvUnLock(pipe);
          }
        } else {
          jsError("Destination object does not implement the required write(buffer, length, position) method.");
        }
      } else {
        jsError("Source object does not implement the required read(buffer, length, position) method.");
      }
      jsvUnLock(ReadFunc);
      jsvUnLock(WriteFunc);
    }
    jsvUnLock(arr);
  }
}

