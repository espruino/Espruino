//try {
  try {
    throw "Error (it's ok, we expected this error)";
  } finally {
    console.log("Finally block executed as expected");
    result = 1;
  }
  console.log("ERROR! exception uncaught so this print shouldn't be executed");
  result = 0;
  try {
    console.log("ERROR! exception uncaught so this try shouldn't be executed");
    result = 0;
  } catch (e) {
    console.log("ERROR! exception uncaught so this catch shouldn't be executed");
    result = 0;
  } finally {
    console.log("ERROR! exception uncaught so this finally shouldn't be executed");  
    result = 0;
  }
} catch (e) {
  if (e!=="Error (it's ok, we expected this error)") {
    console.log("Exception doesn't match");
    result = 0;
  }
}
