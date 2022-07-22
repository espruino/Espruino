#!/usr/bin/node

const fs = require("fs");

function indent(string) {
  return string
    .split("\n")
    .map((line) => (line ? "  " + line : line))
    .join("\n");
}

function getParameterDescription(description) {
  return !description
    ? ""
    : typeof description === "string"
    ? description
    : description.join("\n");
}

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
              `@param {(${getArguments(
                object
              )}) => void} callback - A function that is executed when the event occurs.${
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

function getBasicType(type) {
  if (!type) return "any";
  if (["int", "float", "int32"].includes(type)) return "number";
  if (type == "pin") return "Pin";
  if (type == "bool") return "boolean";
  if (type == "JsVarArray") return "any";
  if (type == "JsVar") return "any";
  if (type == "Array") return "any[]";
  // if (type == "Promise") return "Promise<any>";
  return type;
}

function getArguments(object) {
  let args = [];
  if ("params" in object)
    args = object.params.map((param) => {
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
  return args.join(", ");
}

function getDeclaration(object, c) {
  if (object.type === "event") {
    if (c) {
      return `on(event: "${object.name}", callback: (${getArguments(
        object
      )}) => void): void;`;
    } else {
      return `function on(event: "${object.name}", callback: (${getArguments(
        object
      )}) => void): void;`;
    }
  } else if (
    ["function", "method", "staticmethod", "constructor"].includes(object.type)
  ) {
    // function
    const name = object.type === "constructor" ? "new" : object.name;
    let returnValue = "any";
    if ("return_object" in object)
      returnValue = getBasicType(object.return_object);
    else if ("return" in object) returnValue = getBasicType(object.return[0]);
    if (c) {
      return `${name}(${getArguments(object)}): ${returnValue};`;
    } else {
      return `function ${name}(${getArguments(object)}): ${returnValue};`;
    }
  } else {
    // property
    const type =
      object.type === "object"
        ? object.instanceof
        : getBasicType(object.return_object || object.return[0]);
    if (c) {
      return `${object.name}: ${type};`;
    } else {
      return `const ${object.name}: ${type};`;
    }
  }
}

require("./common.js").readAllWrapperFiles(function (objects) {
  const classes = {};
  const globals = [];

  // Handle classes and libraries first
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

  function getClass(c) {
    if (!classes[c]) classes[c] = { staticProperties: [], prototype: [] };
    return classes[c];
  }

  // Handle contents
  objects.forEach(function (object) {
    if (["include"].includes(object.type)) {
    } else if (["class", "object", "library"].includes(object.type)) {
    } else if (["init", "idle", "kill"].includes(object.type)) {
    } else if (["event"].includes(object.type)) {
      getClass(object.class).staticProperties.push(object);
    } else if (object.type === "constructor") {
      getClass(object.class).cons = object;
    } else if (["staticproperty", "staticmethod"].includes(object.type)) {
      getClass(object.class).staticProperties.push(object);
    } else if (["property", "method"].includes(object.type)) {
      getClass(object.class)["prototype"].push(object);
    } else if (
      ["function", "letiable", "object", "variable"].includes(object.type)
    ) {
      globals.push(object);
    } else console.warn("Unknown type " + object.type + " for ", object);
  });

  const file =
    "// NOTE: This file has been automatically generated.\n\n" +
    '/// <reference path="other.d.ts" />\n\n' +
    "// CLASSES\n\n" +
    Object.entries(classes)
      .filter(([name, c]) => !c.library)
      .map(([name, c]) =>
        name in global
          ? // builtin class (String, Boolean, etc)
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
            `\n}\n\ninterface ${name}${name === "Array" ? "<T>" : ""} {\n` +
            indent(
              c.prototype
                .map((property) =>
                  `${getDocumentation(property)}\n${getDeclaration(
                    property,
                    true
                  )}`.trim()
                )
                .join("\n\n")
            ) +
            `\n}\n\n${getDocumentation(
              c.object
            )}\ndeclare const ${name}: ${name}Constructor`
          : // other class
            `${getDocumentation(c.object)}\ndeclare class ${name} {\n` +
            indent(
              c.staticProperties
                .concat([c.cons])
                .filter((property) => property)
                .map((property) =>
                  `${getDocumentation(property)}\nstatic ${getDeclaration(
                    property,
                    true
                  )}`.trim()
                )
                .join("\n\n") +
                c.prototype
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
      .join("\n\n") +
    "\n\n// GLOBALS\n\n" +
    globals
      .map((global) => {
        if (global.name === "require") {
          return `${getDocumentation(global)}
declare function require<T extends keyof Modules>(moduleName: T): Modules[T]
declare function require<
  T extends Exclude<string, keyof Modules>
>(moduleName: T): any`;
        } else {
          return `${getDocumentation(global)}\ndeclare ${getDeclaration(
            global
          )}`;
        }
      })
      .join("\n\n") +
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
    "\n}";
  fs.writeFileSync("../BangleApps/typescript/types/main.d.ts", file);
});
