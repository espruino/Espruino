# Espruino and TypeScript

## Build script

The build script at `scripts/build_types.js` will automatically generate types
from documentation comments. To use it run:

```sh
node scripts/build_types.js
```

This will update the file located at `typescript/types/main.d.ts` in the
BangleApps repository!

**Note**: The script currently only works if this repository and `BangleApps`
are in the same directory on your file system.

## Making manual declarations

The script can automatically generate some TypeScript declarations from the
information in JSON comments, but others have to be done manually. To add a
declaration for a function or variable add a `"typescript"` field to its JSON
comment. Here's an example from `Graphics.drawString`:

```json
"typescript" : "drawString(str: string, x: number, y: number, solid?: boolean): Graphics;"
```

To declare a function with multiple overloads, specify an array of strings:

```json
"typescript" : [
  "drawRect(x1: number, y1: number, x2: number, y2: number): Graphics;",
  "drawRect(rect: { x: number, y: number, x2: number, y2: number } | { x: number, y: number, w: number, h: number }): Graphics;"
]
```

You can also include declarations in comments starting with `TYPESCRIPT`. Use
these to declare types that are reused between functions or variables that for
whatever reason aren't defined in JSON comments.

For example, the following type is used in Bangle.js's `"accel"` event and
`getAccel` method.

```c
/*TYPESCRIPT
type AccelData = {
  x: number;
  y: number;
  z: number;
  diff: number;
  mag: number;
};
*/
```

`Bangle.CLOCK` and `Bangle.strokes` aren't defined in any JSON comments so we do
it manually:

```c
/*TYPESCRIPT{
  "class" : "Bangle"
}
static CLOCK: boolean;
static strokes: undefined | { [key: string]: Unistroke };
*/
```

Note that you can optionally add a JSON object of the form `{ "class" : string }`
to add the declarations to a specific class. Otherwise they will be declared
globally.

Finally, also note that you can declare classes as generics where necessary. For
example, in the Array class:

```json
"typescript" : "Array<T>"
```

Then you can use this generic in the class's properties and methods:

```json
"typescript" : "includes(value: T, startIndex?: number): boolean;"
```

## Guidelines

Here are some guidelines for making manual declarations:

- Only add a TypeScript declaration to a function or variable if the build
  script can't generate it automatically. Check the `main.d.ts` file first to
  see if it isn't already declared correctly. For example, `Bangle.setLCDPower`
  doesn't need a TypeScript declaration since the script correctly declares it
  automatically.

- The build script can detect optional arguments if their description starts
  with `"[optional]"`. If adding this keyword saves having to declare the
  function manually, add it!

- Try to keep argument names the same as the ones specified in the `"param"`
  field. In cases that this isn't possible (with reserved keywords such as
  `function` or `var`) you'll have to change the name a little (for example,
  `callback` or `variable`).

- In the `"typescript"` field do not add the `static` keyword to static methods
  and properties - the build script will add it automatically.
