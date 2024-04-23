#pragma once

inline const char* g_envCode = R"(
function defineStream(name, getter) {
  Object.defineProperty(process, name, {
    configurable: true,
    enumerable: true,
    get: getter
  });
}

defineStream('stdin', getStdin);

let stdin;

function getStdin() {
  if (stdin) return stdin;
  const fd = 0;

  const { Readable } = require('stream');
  stdin = new Readable({ read() {} });
  stdin.push(null);

  stdin.fd = 0;

  return stdin;
}

//internals can't be accessed anymore even with --expose-internals, patch for this is added in libnode build
const { addBuiltinLibsToObject } = require('internal/modules/helpers');
addBuiltinLibsToObject(global);

const Module = require('module');

const m = new Module('dummy.js');
m.filename = Citizen.getResourcePath() + '/dummy.js';
m.paths = Module._nodeModulePaths(Citizen.getResourcePath() + '/');

const script = 'module.exports = {require};';
const result = m._compile(script, 'dummy-wrapper');

global.require = m.exports.require;
)";
