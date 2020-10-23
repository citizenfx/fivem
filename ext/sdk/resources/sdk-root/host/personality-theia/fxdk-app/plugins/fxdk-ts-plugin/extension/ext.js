var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
  if (k2 === undefined) k2 = k;
  Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
  if (k2 === undefined) k2 = k;
  o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
  Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
  o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
  if (mod && mod.__esModule) return mod;
  var result = {};
  if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
  __setModuleDefault(result, mod);
  return result;
};

Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = void 0;
const vscode = __importStar(require("vscode"));
async function activate(context) {
  console.log('HUUUH: No ts extension');
  // Get the TS extension
  const tsExtension = vscode.extensions.getExtension('vscode.typescript-language-features');
  if (!tsExtension) {
    console.log('HUUUH: No ts extension');
    return;
  }
  await tsExtension.activate();
  // Get the API from the TS extension
  if (!tsExtension.exports || !tsExtension.exports.getAPI) {
    console.log('HUUUH: No exports or no getAPI of tsExtension');
    return;
  }
  const api = tsExtension.exports.getAPI(0);
  if (!api) {
    console.log('HUUUH: No API from tsExt');
    return;
  }
  console.log('HUUUH: configuring custom plugin!');
  // Configure the 'my-typescript-plugin-id' plugin
  api.configurePlugin('actual-fxdk-plugin', {
    yay: true,
  });
}
exports.activate = activate;
