#!/usr/bin/node

/**
 * Add two spaces at the beginning of every line.
 * @param {string} string - The string to indent.
 * @returns {string} The indented string.
 */
function indent(string) {
  return string
    .split("\n")
    .map((line) => (line ? "  " + line : line))
    .join("\n");
}

/**
 * Return the parameter's description.
 * @param {string | string[]} - The description from the JSON comment.
 * @returns {string} The description.
 */
function getParameterDescription(description) {
  return !description
    ? ""
    : typeof description === "string"
    ? description
    : description.join("\n");
}

/**
 * Return the documentation of the function or variable.
 * @param {object} object - The object holding the function or variable.
 * @returns {string} The object's documentation.
 */
function getDocumentation(object) {
  // See https://jsdoc.app/ for how JSDoc comments are formatted
  if (!object) return "";
  return (
    "/**\n" +
    object
      .getDescription()
      .split("\n")
      .filter((line) => line)
      .map((line) => line)
      .concat(object.type === "constructor" ? ["@constructor"] : [])
      .concat(
        object.type === "event"
          ? [
              "@param {string} event - The event to listen to.",
              `@param {${getArguments(
                object
              )} => void} callback - A function that is executed when the event occurs.${
                object.params ? " Its arguments are:" : ""
              }`,
            ].concat(
              object.params
                ? object.params.map(([name, _, description]) =>
                    `* \`${name}\` ${getParameterDescription(
                      description
                    )}`.split("\n")
                  )
                : []
            )
          : object.params
          ? [""].concat(
              object.params
                .map(([name, type, description]) => {
                  const desc = getParameterDescription(description);
                  return (
                    "@param {" +
                    getBasicType(type) +
                    "} " +
                    (desc.startsWith("[optional]") ? "[" + name + "]" : name) +
                    (!description
                      ? ""
                      : typeof description === "string"
                      ? " - " + desc
                      : "\n" + desc)
                  ).split("\n");
                })
                .flat(1)
            )
          : []
      )
      .concat(
        object.return
          ? [
              `@returns {${getBasicType(object.return[0])}} ${
                object.return[1] || ""
              }`,
            ]
          : []
      )
      .concat([`@url ${object.getURL()}`])
      .map((line) => (" * " + line).trimEnd())
      .join("\n") +
    "\n */"
  );
}

/**
 * Convert a basic type to its corresponding TypeScript type.
 * A "basic type" is any but an object or a function.
 * @param {string} type - The basic type.
 * @returns {string} The TypeScript type.
 */
function getBasicType(type) {
  if (!type) return "any";
  if (["int", "float", "int32"].includes(type)) return "number";
  if (type == "pin") return "Pin";
  if (type == "String") return "string";
  if (type == "bool") return "boolean";
  if (type == "JsVarArray") return "any";
  if (type == "JsVar") return "any";
  if (type == "Array") return "any[]";
  if (type == "Promise") return "Promise<void>";
  return type;
}

/**
 * Return the arguments of the method in TypeScript format.
 * @param {object} method - The object containing the method's data.
 * @returns {string} The argument list including brackets.
 */
function getArguments(method) {
  let args = [];
  if ("params" in method)
    args = method.params.map((param) => {
      // hack because digitalRead/Write can also take arrays/objects (but most use cases are Pins)
      if (param[0] == "pin" && param[1] == "JsLet") param[1] = "Pin";
      if (param[0] === "function") param[0] = "func";
      if (param[0] === "var") param[0] = "variable";
      let doc = typeof param[2] === "string" ? param[2] : param[2].join("\n");
      let optional = doc && doc.startsWith("[optional]");
      let rest = param[1] === "JsVarArray";
      return (
        (rest ? "..." : "") +
        param[0] +
        (optional ? "?" : "") +
        ": " +
        getBasicType(param[1]) +
        (rest ? "[]" : "")
      );
    });
  return "(" + args.join(", ") + ")";
}

/**
 * Return the return type of the method in TypeScript format.
 * @param {object} method - The object containing the method's data.
 * @returns {string} The return type.
 */
function getReturnType(method) {
  if ("return_object" in method) return getBasicType(method.return_object);
  if ("return" in method) return getBasicType(method.return[0]);
  return "void";
}

/**
 * Return the declaration of a function or variable.
 * @param {object} object - The object containing the function or variable's data.
 * @param {boolean} [property] - True if the object is a property of a class, etc.
 * @returns {string} The function or variable's declaration.
 */
function getDeclaration(object, property) {
  if ("typescript" in object)
    return typeof object.typescript === "string"
      ? object.typescript
      : object.typescript.join("\n");
  if (object.type === "event") {
    if (property) {
      return `on(event: "${object.name}", callback: ${getArguments(
        object
      )} => void): void;`;
    } else {
      return `function on(event: "${object.name}", callback: ${getArguments(
        object
      )} => void): void;`;
    }
  } else if (
    ["function", "method", "staticmethod", "constructor"].includes(object.type)
  ) {
    // function
    const name = object.type === "constructor" ? "new" : object.name;
    if (property) {
      return `${name}${getArguments(object)}: ${getReturnType(object)};`;
    } else {
      return `declare function ${name}${getArguments(object)}: ${getReturnType(
        object
      )};`;
    }
  } else {
    // property
    const type =
      object.type === "object"
        ? object.instanceof
        : getBasicType(object.return_object || object.return[0]);
    if (property) {
      return `${object.name}: ${type};`;
    } else {
      return `declare const ${object.name}: ${type};`;
    }
  }
}

/**
 * Return classes and libraries.
 * @param {object[]} objects - The list of objects.
 * @returns {object}
 * An object with class names as keys and the following as values:
 * {
 *   library?: true, // whether it's a library or a class
 *   object?, // the object containing its data
 *   staticProperties: [], // a list of its static properties
 *   prototype: [], // a list of the prototype's properties
 *   cons?: // the class's constructor
 * }
 */
function getClasses(objects) {
  const classes = {};
  objects.forEach(function (object) {
    if (object.type == "class" || object.type == "library") {
      classes[object.class] = {
        library: object.type === "library",
        object,
        staticProperties: [],
        prototype: [],
      };
    }
  });
  return classes;
}

/**
 * Return all the objects in an organised structure, so class are
 * found inside their corresponding classes.
 * @param {object[]} objects - The list of objects.
 * @returns {[object, object[]]}
 * An array. The first item is the classes object (see `getClasses`),
 * and the second is an array of global objects.
 */
function getAll(objects) {
  const classes = getClasses(objects);
  const globals = [];

  /**
   * @param {string} c - The name of the class.
   * @returns {object}
   * The class with the corresponding name (see `getClasses` for its
   * contents), or a new one if it doesn't exist.
   */
  function getClass(c) {
    if (!classes[c]) classes[c] = { staticProperties: [], prototype: [] };
    return classes[c];
  }

  objects.forEach(function (object) {
    if (["class", "object", "library"].includes(object.type)) {
      // already handled in `getClases`
    } else if (["include", "init", "idle", "kill"].includes(object.type)) {
      // internal
    } else if (object.type === "constructor") {
      // set as constructor
      getClass(object.class).cons = object;
    } else if (
      ["event", "staticproperty", "staticmethod"].includes(object.type)
    ) {
      // add to static properties
      getClass(object.class).staticProperties.push(object);
    } else if (["property", "method"].includes(object.type)) {
      // add to prototype
      getClass(object.class)["prototype"].push(object);
    } else if (
      ["function", "letiable", "object", "variable", "typescript"].includes(
        object.type
      )
    ) {
      // add to globals
      globals.push(object);
    } else console.warn("Unknown type " + object.type + " for ", object);
  });
  return [classes, globals];
}

/**
 * Return the declarations of custom types.
 * @param {object[]} types - The list of types defined in comments.
 * Of the form { declaration: string, implementation: string }.
 * @returns {string} The joined declarations.
 */
function getTypeDeclarations(types) {
  return (
    "// TYPES\n" +
    types
      .filter((type) => !type.class)
      .map((type) =>
        type.declaration.replace(/\\\//g, "/").replace(/\\\\/g, "\\")
      )
      .join("")
  );
}

/**
 * Get the declaration of a builtin class, that is, that exists in
 * vanilla JavaScript, e.g. String, Array.
 * @param {string} name - The class's name.
 * @param {object} c - The class's data.
 * @param {object[]} types
 * @returns {string} The class's declaration.
 */
function getBuiltinClassDeclaration(name, c, types) {
  return (
    `interface ${name}Constructor {\n` +
    indent(
      c.staticProperties
        .concat([c.cons])
        .filter((property) => property)
        .map((property) =>
          `${getDocumentation(property)}\n${getDeclaration(
            property,
            true
          )}`.trim()
        )
        .join("\n\n")
    ) +
    `\n}\n\n` +
    (name.endsWith("Array") && !name.startsWith("Array") // is a typed array?
      ? `type ${name} = ArrayBufferView<${name}>;\n`
      : `${c.object?.typescript || "interface " + name} {\n` +
        indent(
          c.prototype
            .map((property) =>
              `${getDocumentation(property)}\n${getDeclaration(
                property,
                true
              )}`.trim()
            )
            .concat(name === "Array" ? ["[index: number]: T"] : [])
            .concat(types.map((type) => type.declaration))
            .join("\n\n")
        ) +
        `\n}\n\n${getDocumentation(c.object)}`) +
    `\ndeclare const ${name}: ${name}Constructor`
  );
}

/**
 * Get the declaration of a class that is not builtin.
 * @param {string} name - The class's name.
 * @param {object} c - The class's data.
 * @param {object[]} types
 * @returns {string} The class's declaration.
 */
function getOtherClassDeclaration(name, c, types) {
  return (
    `${getDocumentation(c.object)}\ndeclare class ${
      c.object?.typescript || name
    } {\n` +
    indent(
      c.staticProperties
        .concat([c.cons])
        .filter((property) => property)
        .map((property) =>
          `${getDocumentation(property)}\n${getDeclaration(property, true)
            .split("\n")
            .map((dec) => "static " + dec)
            .join("\n")}`.trim()
        )
        .join("\n\n") +
        "\n\n" +
        c.prototype
          .map((property) =>
            `${getDocumentation(property)}\n${getDeclaration(
              property,
              true
            )}`.trim()
          )
          .concat(name === "ArrayBufferView" ? ["[index: number]: number"] : [])
          .concat(types.map((type) => type.declaration))
          .join("\n\n")
    ) +
    "\n}"
  );
}

/**
 * Return the class declarations (not including libraries).
 * @param {object} classes - The object of classes (see `getClasses`).
 * @param {object[]} types
 * @returns {string} The class declarations.
 */
function getClassDeclarations(classes, types) {
  return (
    "\n\n// CLASSES\n\n" +
    Object.entries(classes)
      .filter(([_, c]) => !c.library)
      .map(([name, c]) =>
        name in global
          ? getBuiltinClassDeclaration(
              name,
              c,
              types.filter((type) => type.class === name)
            )
          : getOtherClassDeclaration(
              name,
              c,
              types.filter((type) => type.class === name)
            )
      )
      .join("\n\n")
  );
}

/**
 * Return the global declarations.
 * @param {object[]} globals - The list of global objects.
 * @returns {string} The global declarations.
 */
function getGlobalDeclarations(globals) {
  return (
    "\n\n// GLOBALS\n\n" +
    globals
      .map((global) =>
        global.name === "global"
          ? `declare const global: {\n` +
            indent(
              globals
                .map((global) => `${global.name}: typeof ${global.name};`)
                .concat("[key: string]: any;")
                .join("\n")
            ) +
            "\n}"
          : `${getDocumentation(global)}\n${getDeclaration(global)}`
      )
      .join("\n\n")
  );
}

/**
 * Return the library declarations.
 * @param {object} classes - The object of classes and libraries (see `getClasses`).
 * @returns {string} The library declarations.
 */
function getLibraryDeclarations(classes) {
  return (
    "\n\n// LIBRARIES\n\ntype Libraries = {\n" +
    indent(
      Object.entries(classes)
        .filter(([_, c]) => c.library)
        .map(
          ([name, library]) =>
            `${getDocumentation(library.object)}\n${name}: {\n` +
            indent(
              library.staticProperties
                .map((property) =>
                  `${getDocumentation(property)}\n${getDeclaration(
                    property,
                    true
                  )}`.trim()
                )
                .join("\n\n")
            ) +
            "\n}"
        )
        .join("\n\n")
    ) +
    "\n}"
  );
}

/**
 * Build TypeScript declarations from the source code's comments.
 * @returns {Promise<string>} Promise that is resolved with the contents of the file to write.
 */
function buildTypes() {
  return new Promise((resolve) => {
    require("./common.js").readAllWrapperFiles(function (objects, types) {
      const [classes, globals] = getAll(objects, types);

      resolve(
        "// NOTE: This file has been automatically generated.\n\n" +
          '/// <reference path="other.d.ts" />\n\n' +
          getTypeDeclarations(types) +
          getClassDeclarations(classes, types) +
          getGlobalDeclarations(globals) +
          getLibraryDeclarations(classes)
      );
    });
  });
}

buildTypes().then((content) =>
  require("fs").writeFileSync(
    __dirname + "/../../BangleApps/typescript/types/main.d.ts",
    content
  )
);
