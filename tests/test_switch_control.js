//https://github.com/espruino/Espruino/issues/2339

(() => {
	let i = 0;
	while (i < 1) {
		console.log("i=", i);
		i++;
		switch (i) {
			case 1:
				continue; // this is crashing
		}
	}
})();

(() => {
	let i = 0;
	for (let i = 0; i < 1; i++) {
		console.log("i=", i);
		switch (i) {
			case 0:
				continue; // this is crashing
		}
	}
})();

(() => {
	let i = 0;
	while (i < 1) {
		console.log("i=", i);
		i++;
		switch (i) {
			default:
				continue;
		}
		throw new Error("this should never be logged"); // this is still being logged in espruino 
	}
})();

(() => {
	let i = 0;
	for (let i = 0; i < 1; i++) {
		console.log("i=", i);
		switch (i) {
			default:
				continue;
		}
		throw new Error("this should never be logged"); // this is still being logged in espruino 
	}
})();

(() => {
  switch ("continue") {
    case "continue": print("Correct");break;
    default: throw Error("Not correct");
  }
})();


/* if there was a problem earlier, an error
should have occurred and we won't get here */
result = 1;
