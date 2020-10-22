/*!-----------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Version: 0.20.0(b0b47123a5da83d42c2675f2bfff5bb9f1b2673c)
 * Released under the MIT license
 * https://github.com/Microsoft/vscode/blob/master/LICENSE.txt
 *-----------------------------------------------------------*/

(function() {
var __m = ["exports","require","vs/editor/common/core/position","vs/base/common/errors","vs/editor/common/core/range","vs/base/common/platform","vs/base/common/lifecycle","vs/base/common/uint","vs/base/common/functional","vs/base/common/types","vs/base/common/diff/diff","vs/base/common/uri","vs/base/common/iterator","vs/base/common/event","vs/base/common/diff/diffChange","vs/base/common/strings","vs/base/common/keyCodes","vs/base/common/hash","vs/base/common/linkedList","vs/editor/common/model/mirrorTextModel","vs/editor/common/core/characterClassifier","vs/base/common/arrays","vs/base/common/cancellation","vs/editor/common/core/selection","vs/editor/common/core/token","vs/editor/common/diff/diffComputer","vs/editor/common/model/wordHelper","vs/editor/common/modes/linkComputer","vs/editor/common/modes/supports/inplaceReplaceSupport","vs/editor/common/standalone/standaloneEnums","vs/editor/common/standalone/standaloneBase","vs/editor/common/viewModel/prefixSumComputer","vs/editor/common/standalone/promise-polyfill/polyfill","vs/base/common/worker/simpleWorker","vs/editor/common/services/editorSimpleWorker"];
var __M = function(deps) {
  var result = [];
  for (var i = 0, len = deps.length; i < len; i++) {
    result[i] = __m[deps[i]];
  }
  return result;
};
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
'use strict';
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 * Please make sure to make edits in the .ts file at https://github.com/Microsoft/vscode-loader/
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *---------------------------------------------------------------------------------------------
 *--------------------------------------------------------------------------------------------*/
var _amdLoaderGlobal = this;
var _commonjsGlobal = typeof global === 'object' ? global : {};
var AMDLoader;
(function (AMDLoader) {
    AMDLoader.global = _amdLoaderGlobal;
    var Environment = /** @class */ (function () {
        function Environment() {
            this._detected = false;
            this._isWindows = false;
            this._isNode = false;
            this._isElectronRenderer = false;
            this._isWebWorker = false;
        }
        Object.defineProperty(Environment.prototype, "isWindows", {
            get: function () {
                this._detect();
                return this._isWindows;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(Environment.prototype, "isNode", {
            get: function () {
                this._detect();
                return this._isNode;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(Environment.prototype, "isElectronRenderer", {
            get: function () {
                this._detect();
                return this._isElectronRenderer;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(Environment.prototype, "isWebWorker", {
            get: function () {
                this._detect();
                return this._isWebWorker;
            },
            enumerable: true,
            configurable: true
        });
        Environment.prototype._detect = function () {
            if (this._detected) {
                return;
            }
            this._detected = true;
            this._isWindows = Environment._isWindows();
            this._isNode = (typeof module !== 'undefined' && !!module.exports);
            this._isElectronRenderer = (typeof process !== 'undefined' && typeof process.versions !== 'undefined' && typeof process.versions.electron !== 'undefined' && process.type === 'renderer');
            this._isWebWorker = (typeof AMDLoader.global.importScripts === 'function');
        };
        Environment._isWindows = function () {
            if (typeof navigator !== 'undefined') {
                if (navigator.userAgent && navigator.userAgent.indexOf('Windows') >= 0) {
                    return true;
                }
            }
            if (typeof process !== 'undefined') {
                return (process.platform === 'win32');
            }
            return false;
        };
        return Environment;
    }());
    AMDLoader.Environment = Environment;
})(AMDLoader || (AMDLoader = {}));
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var AMDLoader;
(function (AMDLoader) {
    var LoaderEvent = /** @class */ (function () {
        function LoaderEvent(type, detail, timestamp) {
            this.type = type;
            this.detail = detail;
            this.timestamp = timestamp;
        }
        return LoaderEvent;
    }());
    AMDLoader.LoaderEvent = LoaderEvent;
    var LoaderEventRecorder = /** @class */ (function () {
        function LoaderEventRecorder(loaderAvailableTimestamp) {
            this._events = [new LoaderEvent(1 /* LoaderAvailable */, '', loaderAvailableTimestamp)];
        }
        LoaderEventRecorder.prototype.record = function (type, detail) {
            this._events.push(new LoaderEvent(type, detail, AMDLoader.Utilities.getHighPerformanceTimestamp()));
        };
        LoaderEventRecorder.prototype.getEvents = function () {
            return this._events;
        };
        return LoaderEventRecorder;
    }());
    AMDLoader.LoaderEventRecorder = LoaderEventRecorder;
    var NullLoaderEventRecorder = /** @class */ (function () {
        function NullLoaderEventRecorder() {
        }
        NullLoaderEventRecorder.prototype.record = function (type, detail) {
            // Nothing to do
        };
        NullLoaderEventRecorder.prototype.getEvents = function () {
            return [];
        };
        NullLoaderEventRecorder.INSTANCE = new NullLoaderEventRecorder();
        return NullLoaderEventRecorder;
    }());
    AMDLoader.NullLoaderEventRecorder = NullLoaderEventRecorder;
})(AMDLoader || (AMDLoader = {}));
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var AMDLoader;
(function (AMDLoader) {
    var Utilities = /** @class */ (function () {
        function Utilities() {
        }
        /**
         * This method does not take care of / vs \
         */
        Utilities.fileUriToFilePath = function (isWindows, uri) {
            uri = decodeURI(uri).replace(/%23/g, '#');
            if (isWindows) {
                if (/^file:\/\/\//.test(uri)) {
                    // This is a URI without a hostname => return only the path segment
                    return uri.substr(8);
                }
                if (/^file:\/\//.test(uri)) {
                    return uri.substr(5);
                }
            }
            else {
                if (/^file:\/\//.test(uri)) {
                    return uri.substr(7);
                }
            }
            // Not sure...
            return uri;
        };
        Utilities.startsWith = function (haystack, needle) {
            return haystack.length >= needle.length && haystack.substr(0, needle.length) === needle;
        };
        Utilities.endsWith = function (haystack, needle) {
            return haystack.length >= needle.length && haystack.substr(haystack.length - needle.length) === needle;
        };
        // only check for "?" before "#" to ensure that there is a real Query-String
        Utilities.containsQueryString = function (url) {
            return /^[^\#]*\?/gi.test(url);
        };
        /**
         * Does `url` start with http:// or https:// or file:// or / ?
         */
        Utilities.isAbsolutePath = function (url) {
            return /^((http:\/\/)|(https:\/\/)|(file:\/\/)|(\/))/.test(url);
        };
        Utilities.forEachProperty = function (obj, callback) {
            if (obj) {
                var key = void 0;
                for (key in obj) {
                    if (obj.hasOwnProperty(key)) {
                        callback(key, obj[key]);
                    }
                }
            }
        };
        Utilities.isEmpty = function (obj) {
            var isEmpty = true;
            Utilities.forEachProperty(obj, function () {
                isEmpty = false;
            });
            return isEmpty;
        };
        Utilities.recursiveClone = function (obj) {
            if (!obj || typeof obj !== 'object') {
                return obj;
            }
            var result = Array.isArray(obj) ? [] : {};
            Utilities.forEachProperty(obj, function (key, value) {
                if (value && typeof value === 'object') {
                    result[key] = Utilities.recursiveClone(value);
                }
                else {
                    result[key] = value;
                }
            });
            return result;
        };
        Utilities.generateAnonymousModule = function () {
            return '===anonymous' + (Utilities.NEXT_ANONYMOUS_ID++) + '===';
        };
        Utilities.isAnonymousModule = function (id) {
            return Utilities.startsWith(id, '===anonymous');
        };
        Utilities.getHighPerformanceTimestamp = function () {
            if (!this.PERFORMANCE_NOW_PROBED) {
                this.PERFORMANCE_NOW_PROBED = true;
                this.HAS_PERFORMANCE_NOW = (AMDLoader.global.performance && typeof AMDLoader.global.performance.now === 'function');
            }
            return (this.HAS_PERFORMANCE_NOW ? AMDLoader.global.performance.now() : Date.now());
        };
        Utilities.NEXT_ANONYMOUS_ID = 1;
        Utilities.PERFORMANCE_NOW_PROBED = false;
        Utilities.HAS_PERFORMANCE_NOW = false;
        return Utilities;
    }());
    AMDLoader.Utilities = Utilities;
})(AMDLoader || (AMDLoader = {}));
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var AMDLoader;
(function (AMDLoader) {
    function ensureError(err) {
        if (err instanceof Error) {
            return err;
        }
        var result = new Error(err.message || String(err) || 'Unknown Error');
        if (err.stack) {
            result.stack = err.stack;
        }
        return result;
    }
    AMDLoader.ensureError = ensureError;
    ;
    var ConfigurationOptionsUtil = /** @class */ (function () {
        function ConfigurationOptionsUtil() {
        }
        /**
         * Ensure configuration options make sense
         */
        ConfigurationOptionsUtil.validateConfigurationOptions = function (options) {
            function defaultOnError(err) {
                if (err.phase === 'loading') {
                    console.error('Loading "' + err.moduleId + '" failed');
                    console.error(err);
                    console.error('Here are the modules that depend on it:');
                    console.error(err.neededBy);
                    return;
                }
                if (err.phase === 'factory') {
                    console.error('The factory method of "' + err.moduleId + '" has thrown an exception');
                    console.error(err);
                    return;
                }
            }
            options = options || {};
            if (typeof options.baseUrl !== 'string') {
                options.baseUrl = '';
            }
            if (typeof options.isBuild !== 'boolean') {
                options.isBuild = false;
            }
            if (typeof options.paths !== 'object') {
                options.paths = {};
            }
            if (typeof options.config !== 'object') {
                options.config = {};
            }
            if (typeof options.catchError === 'undefined') {
                options.catchError = false;
            }
            if (typeof options.recordStats === 'undefined') {
                options.recordStats = false;
            }
            if (typeof options.urlArgs !== 'string') {
                options.urlArgs = '';
            }
            if (typeof options.onError !== 'function') {
                options.onError = defaultOnError;
            }
            if (!Array.isArray(options.ignoreDuplicateModules)) {
                options.ignoreDuplicateModules = [];
            }
            if (options.baseUrl.length > 0) {
                if (!AMDLoader.Utilities.endsWith(options.baseUrl, '/')) {
                    options.baseUrl += '/';
                }
            }
            if (typeof options.cspNonce !== 'string') {
                options.cspNonce = '';
            }
            if (!Array.isArray(options.nodeModules)) {
                options.nodeModules = [];
            }
            if (options.nodeCachedData && typeof options.nodeCachedData === 'object') {
                if (typeof options.nodeCachedData.seed !== 'string') {
                    options.nodeCachedData.seed = 'seed';
                }
                if (typeof options.nodeCachedData.writeDelay !== 'number' || options.nodeCachedData.writeDelay < 0) {
                    options.nodeCachedData.writeDelay = 1000 * 7;
                }
                if (!options.nodeCachedData.path || typeof options.nodeCachedData.path !== 'string') {
                    var err = ensureError(new Error('INVALID cached data configuration, \'path\' MUST be set'));
                    err.phase = 'configuration';
                    options.onError(err);
                    options.nodeCachedData = undefined;
                }
            }
            return options;
        };
        ConfigurationOptionsUtil.mergeConfigurationOptions = function (overwrite, base) {
            if (overwrite === void 0) { overwrite = null; }
            if (base === void 0) { base = null; }
            var result = AMDLoader.Utilities.recursiveClone(base || {});
            // Merge known properties and overwrite the unknown ones
            AMDLoader.Utilities.forEachProperty(overwrite, function (key, value) {
                if (key === 'ignoreDuplicateModules' && typeof result.ignoreDuplicateModules !== 'undefined') {
                    result.ignoreDuplicateModules = result.ignoreDuplicateModules.concat(value);
                }
                else if (key === 'paths' && typeof result.paths !== 'undefined') {
                    AMDLoader.Utilities.forEachProperty(value, function (key2, value2) { return result.paths[key2] = value2; });
                }
                else if (key === 'config' && typeof result.config !== 'undefined') {
                    AMDLoader.Utilities.forEachProperty(value, function (key2, value2) { return result.config[key2] = value2; });
                }
                else {
                    result[key] = AMDLoader.Utilities.recursiveClone(value);
                }
            });
            return ConfigurationOptionsUtil.validateConfigurationOptions(result);
        };
        return ConfigurationOptionsUtil;
    }());
    AMDLoader.ConfigurationOptionsUtil = ConfigurationOptionsUtil;
    var Configuration = /** @class */ (function () {
        function Configuration(env, options) {
            this._env = env;
            this.options = ConfigurationOptionsUtil.mergeConfigurationOptions(options);
            this._createIgnoreDuplicateModulesMap();
            this._createNodeModulesMap();
            this._createSortedPathsRules();
            if (this.options.baseUrl === '') {
                if (this.options.nodeRequire && this.options.nodeRequire.main && this.options.nodeRequire.main.filename && this._env.isNode) {
                    var nodeMain = this.options.nodeRequire.main.filename;
                    var dirnameIndex = Math.max(nodeMain.lastIndexOf('/'), nodeMain.lastIndexOf('\\'));
                    this.options.baseUrl = nodeMain.substring(0, dirnameIndex + 1);
                }
                if (this.options.nodeMain && this._env.isNode) {
                    var nodeMain = this.options.nodeMain;
                    var dirnameIndex = Math.max(nodeMain.lastIndexOf('/'), nodeMain.lastIndexOf('\\'));
                    this.options.baseUrl = nodeMain.substring(0, dirnameIndex + 1);
                }
            }
        }
        Configuration.prototype._createIgnoreDuplicateModulesMap = function () {
            // Build a map out of the ignoreDuplicateModules array
            this.ignoreDuplicateModulesMap = {};
            for (var i = 0; i < this.options.ignoreDuplicateModules.length; i++) {
                this.ignoreDuplicateModulesMap[this.options.ignoreDuplicateModules[i]] = true;
            }
        };
        Configuration.prototype._createNodeModulesMap = function () {
            // Build a map out of nodeModules array
            this.nodeModulesMap = Object.create(null);
            for (var _i = 0, _a = this.options.nodeModules; _i < _a.length; _i++) {
                var nodeModule = _a[_i];
                this.nodeModulesMap[nodeModule] = true;
            }
        };
        Configuration.prototype._createSortedPathsRules = function () {
            var _this = this;
            // Create an array our of the paths rules, sorted descending by length to
            // result in a more specific -> less specific order
            this.sortedPathsRules = [];
            AMDLoader.Utilities.forEachProperty(this.options.paths, function (from, to) {
                if (!Array.isArray(to)) {
                    _this.sortedPathsRules.push({
                        from: from,
                        to: [to]
                    });
                }
                else {
                    _this.sortedPathsRules.push({
                        from: from,
                        to: to
                    });
                }
            });
            this.sortedPathsRules.sort(function (a, b) {
                return b.from.length - a.from.length;
            });
        };
        /**
         * Clone current configuration and overwrite options selectively.
         * @param options The selective options to overwrite with.
         * @result A new configuration
         */
        Configuration.prototype.cloneAndMerge = function (options) {
            return new Configuration(this._env, ConfigurationOptionsUtil.mergeConfigurationOptions(options, this.options));
        };
        /**
         * Get current options bag. Useful for passing it forward to plugins.
         */
        Configuration.prototype.getOptionsLiteral = function () {
            return this.options;
        };
        Configuration.prototype._applyPaths = function (moduleId) {
            var pathRule;
            for (var i = 0, len = this.sortedPathsRules.length; i < len; i++) {
                pathRule = this.sortedPathsRules[i];
                if (AMDLoader.Utilities.startsWith(moduleId, pathRule.from)) {
                    var result = [];
                    for (var j = 0, lenJ = pathRule.to.length; j < lenJ; j++) {
                        result.push(pathRule.to[j] + moduleId.substr(pathRule.from.length));
                    }
                    return result;
                }
            }
            return [moduleId];
        };
        Configuration.prototype._addUrlArgsToUrl = function (url) {
            if (AMDLoader.Utilities.containsQueryString(url)) {
                return url + '&' + this.options.urlArgs;
            }
            else {
                return url + '?' + this.options.urlArgs;
            }
        };
        Configuration.prototype._addUrlArgsIfNecessaryToUrl = function (url) {
            if (this.options.urlArgs) {
                return this._addUrlArgsToUrl(url);
            }
            return url;
        };
        Configuration.prototype._addUrlArgsIfNecessaryToUrls = function (urls) {
            if (this.options.urlArgs) {
                for (var i = 0, len = urls.length; i < len; i++) {
                    urls[i] = this._addUrlArgsToUrl(urls[i]);
                }
            }
            return urls;
        };
        /**
         * Transform a module id to a location. Appends .js to module ids
         */
        Configuration.prototype.moduleIdToPaths = function (moduleId) {
            if (this.nodeModulesMap[moduleId] === true) {
                // This is a node module...
                if (this.isBuild()) {
                    // ...and we are at build time, drop it
                    return ['empty:'];
                }
                else {
                    // ...and at runtime we create a `shortcut`-path
                    return ['node|' + moduleId];
                }
            }
            var result = moduleId;
            var results;
            if (!AMDLoader.Utilities.endsWith(result, '.js') && !AMDLoader.Utilities.isAbsolutePath(result)) {
                results = this._applyPaths(result);
                for (var i = 0, len = results.length; i < len; i++) {
                    if (this.isBuild() && results[i] === 'empty:') {
                        continue;
                    }
                    if (!AMDLoader.Utilities.isAbsolutePath(results[i])) {
                        results[i] = this.options.baseUrl + results[i];
                    }
                    if (!AMDLoader.Utilities.endsWith(results[i], '.js') && !AMDLoader.Utilities.containsQueryString(results[i])) {
                        results[i] = results[i] + '.js';
                    }
                }
            }
            else {
                if (!AMDLoader.Utilities.endsWith(result, '.js') && !AMDLoader.Utilities.containsQueryString(result)) {
                    result = result + '.js';
                }
                results = [result];
            }
            return this._addUrlArgsIfNecessaryToUrls(results);
        };
        /**
         * Transform a module id or url to a location.
         */
        Configuration.prototype.requireToUrl = function (url) {
            var result = url;
            if (!AMDLoader.Utilities.isAbsolutePath(result)) {
                result = this._applyPaths(result)[0];
                if (!AMDLoader.Utilities.isAbsolutePath(result)) {
                    result = this.options.baseUrl + result;
                }
            }
            return this._addUrlArgsIfNecessaryToUrl(result);
        };
        /**
         * Flag to indicate if current execution is as part of a build.
         */
        Configuration.prototype.isBuild = function () {
            return this.options.isBuild;
        };
        /**
         * Test if module `moduleId` is expected to be defined multiple times
         */
        Configuration.prototype.isDuplicateMessageIgnoredFor = function (moduleId) {
            return this.ignoreDuplicateModulesMap.hasOwnProperty(moduleId);
        };
        /**
         * Get the configuration settings for the provided module id
         */
        Configuration.prototype.getConfigForModule = function (moduleId) {
            if (this.options.config) {
                return this.options.config[moduleId];
            }
        };
        /**
         * Should errors be caught when executing module factories?
         */
        Configuration.prototype.shouldCatchError = function () {
            return this.options.catchError;
        };
        /**
         * Should statistics be recorded?
         */
        Configuration.prototype.shouldRecordStats = function () {
            return this.options.recordStats;
        };
        /**
         * Forward an error to the error handler.
         */
        Configuration.prototype.onError = function (err) {
            this.options.onError(err);
        };
        return Configuration;
    }());
    AMDLoader.Configuration = Configuration;
})(AMDLoader || (AMDLoader = {}));
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var AMDLoader;
(function (AMDLoader) {
    /**
     * Load `scriptSrc` only once (avoid multiple <script> tags)
     */
    var OnlyOnceScriptLoader = /** @class */ (function () {
        function OnlyOnceScriptLoader(env) {
            this._env = env;
            this._scriptLoader = null;
            this._callbackMap = {};
        }
        OnlyOnceScriptLoader.prototype.load = function (moduleManager, scriptSrc, callback, errorback) {
            var _this = this;
            if (!this._scriptLoader) {
                this._scriptLoader = (this._env.isWebWorker
                    ? new WorkerScriptLoader()
                    : this._env.isNode
                        ? new NodeScriptLoader(this._env)
                        : new BrowserScriptLoader());
            }
            var scriptCallbacks = {
                callback: callback,
                errorback: errorback
            };
            if (this._callbackMap.hasOwnProperty(scriptSrc)) {
                this._callbackMap[scriptSrc].push(scriptCallbacks);
                return;
            }
            this._callbackMap[scriptSrc] = [scriptCallbacks];
            this._scriptLoader.load(moduleManager, scriptSrc, function () { return _this.triggerCallback(scriptSrc); }, function (err) { return _this.triggerErrorback(scriptSrc, err); });
        };
        OnlyOnceScriptLoader.prototype.triggerCallback = function (scriptSrc) {
            var scriptCallbacks = this._callbackMap[scriptSrc];
            delete this._callbackMap[scriptSrc];
            for (var i = 0; i < scriptCallbacks.length; i++) {
                scriptCallbacks[i].callback();
            }
        };
        OnlyOnceScriptLoader.prototype.triggerErrorback = function (scriptSrc, err) {
            var scriptCallbacks = this._callbackMap[scriptSrc];
            delete this._callbackMap[scriptSrc];
            for (var i = 0; i < scriptCallbacks.length; i++) {
                scriptCallbacks[i].errorback(err);
            }
        };
        return OnlyOnceScriptLoader;
    }());
    var BrowserScriptLoader = /** @class */ (function () {
        function BrowserScriptLoader() {
        }
        /**
         * Attach load / error listeners to a script element and remove them when either one has fired.
         * Implemented for browssers supporting HTML5 standard 'load' and 'error' events.
         */
        BrowserScriptLoader.prototype.attachListeners = function (script, callback, errorback) {
            var unbind = function () {
                script.removeEventListener('load', loadEventListener);
                script.removeEventListener('error', errorEventListener);
            };
            var loadEventListener = function (e) {
                unbind();
                callback();
            };
            var errorEventListener = function (e) {
                unbind();
                errorback(e);
            };
            script.addEventListener('load', loadEventListener);
            script.addEventListener('error', errorEventListener);
        };
        BrowserScriptLoader.prototype.load = function (moduleManager, scriptSrc, callback, errorback) {
            var script = document.createElement('script');
            script.setAttribute('async', 'async');
            script.setAttribute('type', 'text/javascript');
            this.attachListeners(script, callback, errorback);
            script.setAttribute('src', scriptSrc);
            // Propagate CSP nonce to dynamically created script tag.
            var cspNonce = moduleManager.getConfig().getOptionsLiteral().cspNonce;
            if (cspNonce) {
                script.setAttribute('nonce', cspNonce);
            }
            document.getElementsByTagName('head')[0].appendChild(script);
        };
        return BrowserScriptLoader;
    }());
    var WorkerScriptLoader = /** @class */ (function () {
        function WorkerScriptLoader() {
        }
        WorkerScriptLoader.prototype.load = function (moduleManager, scriptSrc, callback, errorback) {
            try {
                importScripts(scriptSrc);
                callback();
            }
            catch (e) {
                errorback(e);
            }
        };
        return WorkerScriptLoader;
    }());
    var NodeScriptLoader = /** @class */ (function () {
        function NodeScriptLoader(env) {
            this._env = env;
            this._didInitialize = false;
            this._didPatchNodeRequire = false;
        }
        NodeScriptLoader.prototype._init = function (nodeRequire) {
            if (this._didInitialize) {
                return;
            }
            this._didInitialize = true;
            // capture node modules
            this._fs = nodeRequire('fs');
            this._vm = nodeRequire('vm');
            this._path = nodeRequire('path');
            this._crypto = nodeRequire('crypto');
        };
        // patch require-function of nodejs such that we can manually create a script
        // from cached data. this is done by overriding the `Module._compile` function
        NodeScriptLoader.prototype._initNodeRequire = function (nodeRequire, moduleManager) {
            // It is important to check for `nodeCachedData` first and then set `_didPatchNodeRequire`.
            // That's because `nodeCachedData` is set _after_ calling this for the first time...
            var nodeCachedData = moduleManager.getConfig().getOptionsLiteral().nodeCachedData;
            if (!nodeCachedData) {
                return;
            }
            if (this._didPatchNodeRequire) {
                return;
            }
            this._didPatchNodeRequire = true;
            var that = this;
            var Module = nodeRequire('module');
            function makeRequireFunction(mod) {
                var Module = mod.constructor;
                var require = function require(path) {
                    try {
                        return mod.require(path);
                    }
                    finally {
                        // nothing
                    }
                };
                require.resolve = function resolve(request) {
                    return Module._resolveFilename(request, mod);
                };
                require.main = process.mainModule;
                require.extensions = Module._extensions;
                require.cache = Module._cache;
                return require;
            }
            Module.prototype._compile = function (content, filename) {
                // remove shebang and create wrapper function
                var scriptSource = Module.wrap(content.replace(/^#!.*/, ''));
                // create script
                var recorder = moduleManager.getRecorder();
                var cachedDataPath = that._getCachedDataPath(nodeCachedData, filename);
                var options = { filename: filename };
                var hashData;
                try {
                    var data = that._fs.readFileSync(cachedDataPath);
                    hashData = data.slice(0, 16);
                    options.cachedData = data.slice(16);
                    recorder.record(60 /* CachedDataFound */, cachedDataPath);
                }
                catch (_e) {
                    recorder.record(61 /* CachedDataMissed */, cachedDataPath);
                }
                var script = new that._vm.Script(scriptSource, options);
                var compileWrapper = script.runInThisContext(options);
                // run script
                var dirname = that._path.dirname(filename);
                var require = makeRequireFunction(this);
                var args = [this.exports, require, this, filename, dirname, process, _commonjsGlobal, Buffer];
                var result = compileWrapper.apply(this.exports, args);
                // cached data aftermath
                that._handleCachedData(script, scriptSource, cachedDataPath, !options.cachedData, moduleManager);
                that._verifyCachedData(script, scriptSource, cachedDataPath, hashData, moduleManager);
                return result;
            };
        };
        NodeScriptLoader.prototype.load = function (moduleManager, scriptSrc, callback, errorback) {
            var _this = this;
            var opts = moduleManager.getConfig().getOptionsLiteral();
            var nodeRequire = (opts.nodeRequire || AMDLoader.global.nodeRequire);
            var nodeInstrumenter = (opts.nodeInstrumenter || function (c) { return c; });
            this._init(nodeRequire);
            this._initNodeRequire(nodeRequire, moduleManager);
            var recorder = moduleManager.getRecorder();
            if (/^node\|/.test(scriptSrc)) {
                var pieces = scriptSrc.split('|');
                var moduleExports_1 = null;
                try {
                    moduleExports_1 = nodeRequire(pieces[1]);
                }
                catch (err) {
                    errorback(err);
                    return;
                }
                moduleManager.enqueueDefineAnonymousModule([], function () { return moduleExports_1; });
                callback();
            }
            else {
                scriptSrc = AMDLoader.Utilities.fileUriToFilePath(this._env.isWindows, scriptSrc);
                var normalizedScriptSrc_1 = this._path.normalize(scriptSrc);
                var vmScriptPathOrUri_1 = this._getElectronRendererScriptPathOrUri(normalizedScriptSrc_1);
                var wantsCachedData_1 = Boolean(opts.nodeCachedData);
                var cachedDataPath_1 = wantsCachedData_1 ? this._getCachedDataPath(opts.nodeCachedData, scriptSrc) : undefined;
                this._readSourceAndCachedData(normalizedScriptSrc_1, cachedDataPath_1, recorder, function (err, data, cachedData, hashData) {
                    if (err) {
                        errorback(err);
                        return;
                    }
                    var scriptSource;
                    if (data.charCodeAt(0) === NodeScriptLoader._BOM) {
                        scriptSource = NodeScriptLoader._PREFIX + data.substring(1) + NodeScriptLoader._SUFFIX;
                    }
                    else {
                        scriptSource = NodeScriptLoader._PREFIX + data + NodeScriptLoader._SUFFIX;
                    }
                    scriptSource = nodeInstrumenter(scriptSource, normalizedScriptSrc_1);
                    var scriptOpts = { filename: vmScriptPathOrUri_1, cachedData: cachedData };
                    var script = _this._createAndEvalScript(moduleManager, scriptSource, scriptOpts, callback, errorback);
                    _this._handleCachedData(script, scriptSource, cachedDataPath_1, wantsCachedData_1 && !cachedData, moduleManager);
                    _this._verifyCachedData(script, scriptSource, cachedDataPath_1, hashData, moduleManager);
                });
            }
        };
        NodeScriptLoader.prototype._createAndEvalScript = function (moduleManager, contents, options, callback, errorback) {
            var recorder = moduleManager.getRecorder();
            recorder.record(31 /* NodeBeginEvaluatingScript */, options.filename);
            var script = new this._vm.Script(contents, options);
            var ret = script.runInThisContext(options);
            var globalDefineFunc = moduleManager.getGlobalAMDDefineFunc();
            var receivedDefineCall = false;
            var localDefineFunc = function () {
                receivedDefineCall = true;
                return globalDefineFunc.apply(null, arguments);
            };
            localDefineFunc.amd = globalDefineFunc.amd;
            ret.call(AMDLoader.global, moduleManager.getGlobalAMDRequireFunc(), localDefineFunc, options.filename, this._path.dirname(options.filename));
            recorder.record(32 /* NodeEndEvaluatingScript */, options.filename);
            if (receivedDefineCall) {
                callback();
            }
            else {
                errorback(new Error("Didn't receive define call in " + options.filename + "!"));
            }
            return script;
        };
        NodeScriptLoader.prototype._getElectronRendererScriptPathOrUri = function (path) {
            if (!this._env.isElectronRenderer) {
                return path;
            }
            var driveLetterMatch = path.match(/^([a-z])\:(.*)/i);
            if (driveLetterMatch) {
                // windows
                return "file:///" + (driveLetterMatch[1].toUpperCase() + ':' + driveLetterMatch[2]).replace(/\\/g, '/');
            }
            else {
                // nix
                return "file://" + path;
            }
        };
        NodeScriptLoader.prototype._getCachedDataPath = function (config, filename) {
            var hash = this._crypto.createHash('md5').update(filename, 'utf8').update(config.seed, 'utf8').digest('hex');
            var basename = this._path.basename(filename).replace(/\.js$/, '');
            return this._path.join(config.path, basename + "-" + hash + ".code");
        };
        NodeScriptLoader.prototype._handleCachedData = function (script, scriptSource, cachedDataPath, createCachedData, moduleManager) {
            var _this = this;
            if (script.cachedDataRejected) {
                // cached data got rejected -> delete and re-create
                this._fs.unlink(cachedDataPath, function (err) {
                    moduleManager.getRecorder().record(62 /* CachedDataRejected */, cachedDataPath);
                    _this._createAndWriteCachedData(script, scriptSource, cachedDataPath, moduleManager);
                    if (err) {
                        moduleManager.getConfig().onError(err);
                    }
                });
            }
            else if (createCachedData) {
                // no cached data, but wanted
                this._createAndWriteCachedData(script, scriptSource, cachedDataPath, moduleManager);
            }
        };
        // Cached data format: | SOURCE_HASH | V8_CACHED_DATA |
        // -SOURCE_HASH is the md5 hash of the JS source (always 16 bytes)
        // -V8_CACHED_DATA is what v8 produces
        NodeScriptLoader.prototype._createAndWriteCachedData = function (script, scriptSource, cachedDataPath, moduleManager) {
            var _this = this;
            var timeout = Math.ceil(moduleManager.getConfig().getOptionsLiteral().nodeCachedData.writeDelay * (1 + Math.random()));
            var lastSize = -1;
            var iteration = 0;
            var hashData = undefined;
            var createLoop = function () {
                setTimeout(function () {
                    if (!hashData) {
                        hashData = _this._crypto.createHash('md5').update(scriptSource, 'utf8').digest();
                    }
                    var cachedData = script.createCachedData();
                    if (cachedData.length === 0 || cachedData.length === lastSize || iteration >= 5) {
                        return;
                    }
                    lastSize = cachedData.length;
                    _this._fs.writeFile(cachedDataPath, Buffer.concat([hashData, cachedData]), function (err) {
                        if (err) {
                            moduleManager.getConfig().onError(err);
                        }
                        moduleManager.getRecorder().record(63 /* CachedDataCreated */, cachedDataPath);
                        createLoop();
                    });
                }, timeout * (Math.pow(4, iteration++)));
            };
            // with some delay (`timeout`) create cached data
            // and repeat that (with backoff delay) until the
            // data seems to be not changing anymore
            createLoop();
        };
        NodeScriptLoader.prototype._readSourceAndCachedData = function (sourcePath, cachedDataPath, recorder, callback) {
            if (!cachedDataPath) {
                // no cached data case
                this._fs.readFile(sourcePath, { encoding: 'utf8' }, callback);
            }
            else {
                // cached data case: read both files in parallel
                var source_1 = undefined;
                var cachedData_1 = undefined;
                var hashData_1 = undefined;
                var steps_1 = 2;
                var step_1 = function (err) {
                    if (err) {
                        callback(err);
                    }
                    else if (--steps_1 === 0) {
                        callback(undefined, source_1, cachedData_1, hashData_1);
                    }
                };
                this._fs.readFile(sourcePath, { encoding: 'utf8' }, function (err, data) {
                    source_1 = data;
                    step_1(err);
                });
                this._fs.readFile(cachedDataPath, function (err, data) {
                    if (!err && data && data.length > 0) {
                        hashData_1 = data.slice(0, 16);
                        cachedData_1 = data.slice(16);
                        recorder.record(60 /* CachedDataFound */, cachedDataPath);
                    }
                    else {
                        recorder.record(61 /* CachedDataMissed */, cachedDataPath);
                    }
                    step_1(); // ignored: cached data is optional
                });
            }
        };
        NodeScriptLoader.prototype._verifyCachedData = function (script, scriptSource, cachedDataPath, hashData, moduleManager) {
            var _this = this;
            if (!hashData) {
                // nothing to do
                return;
            }
            if (script.cachedDataRejected) {
                // invalid anyways
                return;
            }
            setTimeout(function () {
                // check source hash - the contract is that file paths change when file content
                // change (e.g use the commit or version id as cache path). this check is
                // for violations of this contract.
                var hashDataNow = _this._crypto.createHash('md5').update(scriptSource, 'utf8').digest();
                if (!hashData.equals(hashDataNow)) {
                    moduleManager.getConfig().onError(new Error("FAILED TO VERIFY CACHED DATA, deleting stale '" + cachedDataPath + "' now, but a RESTART IS REQUIRED"));
                    _this._fs.unlink(cachedDataPath, function (err) { return moduleManager.getConfig().onError(err); });
                }
            }, Math.ceil(5000 * (1 + Math.random())));
        };
        NodeScriptLoader._BOM = 0xFEFF;
        NodeScriptLoader._PREFIX = '(function (require, define, __filename, __dirname) { ';
        NodeScriptLoader._SUFFIX = '\n});';
        return NodeScriptLoader;
    }());
    function createScriptLoader(env) {
        return new OnlyOnceScriptLoader(env);
    }
    AMDLoader.createScriptLoader = createScriptLoader;
})(AMDLoader || (AMDLoader = {}));
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var AMDLoader;
(function (AMDLoader) {
    // ------------------------------------------------------------------------
    // ModuleIdResolver
    var ModuleIdResolver = /** @class */ (function () {
        function ModuleIdResolver(fromModuleId) {
            var lastSlash = fromModuleId.lastIndexOf('/');
            if (lastSlash !== -1) {
                this.fromModulePath = fromModuleId.substr(0, lastSlash + 1);
            }
            else {
                this.fromModulePath = '';
            }
        }
        /**
         * Normalize 'a/../name' to 'name', etc.
         */
        ModuleIdResolver._normalizeModuleId = function (moduleId) {
            var r = moduleId, pattern;
            // replace /./ => /
            pattern = /\/\.\//;
            while (pattern.test(r)) {
                r = r.replace(pattern, '/');
            }
            // replace ^./ => nothing
            r = r.replace(/^\.\//g, '');
            // replace /aa/../ => / (BUT IGNORE /../../)
            pattern = /\/(([^\/])|([^\/][^\/\.])|([^\/\.][^\/])|([^\/][^\/][^\/]+))\/\.\.\//;
            while (pattern.test(r)) {
                r = r.replace(pattern, '/');
            }
            // replace ^aa/../ => nothing (BUT IGNORE ../../)
            r = r.replace(/^(([^\/])|([^\/][^\/\.])|([^\/\.][^\/])|([^\/][^\/][^\/]+))\/\.\.\//, '');
            return r;
        };
        /**
         * Resolve relative module ids
         */
        ModuleIdResolver.prototype.resolveModule = function (moduleId) {
            var result = moduleId;
            if (!AMDLoader.Utilities.isAbsolutePath(result)) {
                if (AMDLoader.Utilities.startsWith(result, './') || AMDLoader.Utilities.startsWith(result, '../')) {
                    result = ModuleIdResolver._normalizeModuleId(this.fromModulePath + result);
                }
            }
            return result;
        };
        ModuleIdResolver.ROOT = new ModuleIdResolver('');
        return ModuleIdResolver;
    }());
    AMDLoader.ModuleIdResolver = ModuleIdResolver;
    // ------------------------------------------------------------------------
    // Module
    var Module = /** @class */ (function () {
        function Module(id, strId, dependencies, callback, errorback, moduleIdResolver) {
            this.id = id;
            this.strId = strId;
            this.dependencies = dependencies;
            this._callback = callback;
            this._errorback = errorback;
            this.moduleIdResolver = moduleIdResolver;
            this.exports = {};
            this.error = null;
            this.exportsPassedIn = false;
            this.unresolvedDependenciesCount = this.dependencies.length;
            this._isComplete = false;
        }
        Module._safeInvokeFunction = function (callback, args) {
            try {
                return {
                    returnedValue: callback.apply(AMDLoader.global, args),
                    producedError: null
                };
            }
            catch (e) {
                return {
                    returnedValue: null,
                    producedError: e
                };
            }
        };
        Module._invokeFactory = function (config, strModuleId, callback, dependenciesValues) {
            if (config.isBuild() && !AMDLoader.Utilities.isAnonymousModule(strModuleId)) {
                return {
                    returnedValue: null,
                    producedError: null
                };
            }
            if (config.shouldCatchError()) {
                return this._safeInvokeFunction(callback, dependenciesValues);
            }
            return {
                returnedValue: callback.apply(AMDLoader.global, dependenciesValues),
                producedError: null
            };
        };
        Module.prototype.complete = function (recorder, config, dependenciesValues) {
            this._isComplete = true;
            var producedError = null;
            if (this._callback) {
                if (typeof this._callback === 'function') {
                    recorder.record(21 /* BeginInvokeFactory */, this.strId);
                    var r = Module._invokeFactory(config, this.strId, this._callback, dependenciesValues);
                    producedError = r.producedError;
                    recorder.record(22 /* EndInvokeFactory */, this.strId);
                    if (!producedError && typeof r.returnedValue !== 'undefined' && (!this.exportsPassedIn || AMDLoader.Utilities.isEmpty(this.exports))) {
                        this.exports = r.returnedValue;
                    }
                }
                else {
                    this.exports = this._callback;
                }
            }
            if (producedError) {
                var err = AMDLoader.ensureError(producedError);
                err.phase = 'factory';
                err.moduleId = this.strId;
                this.error = err;
                config.onError(err);
            }
            this.dependencies = null;
            this._callback = null;
            this._errorback = null;
            this.moduleIdResolver = null;
        };
        /**
         * One of the direct dependencies or a transitive dependency has failed to load.
         */
        Module.prototype.onDependencyError = function (err) {
            this._isComplete = true;
            this.error = err;
            if (this._errorback) {
                this._errorback(err);
                return true;
            }
            return false;
        };
        /**
         * Is the current module complete?
         */
        Module.prototype.isComplete = function () {
            return this._isComplete;
        };
        return Module;
    }());
    AMDLoader.Module = Module;
    var ModuleIdProvider = /** @class */ (function () {
        function ModuleIdProvider() {
            this._nextId = 0;
            this._strModuleIdToIntModuleId = new Map();
            this._intModuleIdToStrModuleId = [];
            // Ensure values 0, 1, 2 are assigned accordingly with ModuleId
            this.getModuleId('exports');
            this.getModuleId('module');
            this.getModuleId('require');
        }
        ModuleIdProvider.prototype.getMaxModuleId = function () {
            return this._nextId;
        };
        ModuleIdProvider.prototype.getModuleId = function (strModuleId) {
            var id = this._strModuleIdToIntModuleId.get(strModuleId);
            if (typeof id === 'undefined') {
                id = this._nextId++;
                this._strModuleIdToIntModuleId.set(strModuleId, id);
                this._intModuleIdToStrModuleId[id] = strModuleId;
            }
            return id;
        };
        ModuleIdProvider.prototype.getStrModuleId = function (moduleId) {
            return this._intModuleIdToStrModuleId[moduleId];
        };
        return ModuleIdProvider;
    }());
    var RegularDependency = /** @class */ (function () {
        function RegularDependency(id) {
            this.id = id;
        }
        RegularDependency.EXPORTS = new RegularDependency(0 /* EXPORTS */);
        RegularDependency.MODULE = new RegularDependency(1 /* MODULE */);
        RegularDependency.REQUIRE = new RegularDependency(2 /* REQUIRE */);
        return RegularDependency;
    }());
    AMDLoader.RegularDependency = RegularDependency;
    var PluginDependency = /** @class */ (function () {
        function PluginDependency(id, pluginId, pluginParam) {
            this.id = id;
            this.pluginId = pluginId;
            this.pluginParam = pluginParam;
        }
        return PluginDependency;
    }());
    AMDLoader.PluginDependency = PluginDependency;
    var ModuleManager = /** @class */ (function () {
        function ModuleManager(env, scriptLoader, defineFunc, requireFunc, loaderAvailableTimestamp) {
            if (loaderAvailableTimestamp === void 0) { loaderAvailableTimestamp = 0; }
            this._env = env;
            this._scriptLoader = scriptLoader;
            this._loaderAvailableTimestamp = loaderAvailableTimestamp;
            this._defineFunc = defineFunc;
            this._requireFunc = requireFunc;
            this._moduleIdProvider = new ModuleIdProvider();
            this._config = new AMDLoader.Configuration(this._env);
            this._modules2 = [];
            this._knownModules2 = [];
            this._inverseDependencies2 = [];
            this._inversePluginDependencies2 = new Map();
            this._currentAnnonymousDefineCall = null;
            this._recorder = null;
            this._buildInfoPath = [];
            this._buildInfoDefineStack = [];
            this._buildInfoDependencies = [];
        }
        ModuleManager.prototype.reset = function () {
            return new ModuleManager(this._env, this._scriptLoader, this._defineFunc, this._requireFunc, this._loaderAvailableTimestamp);
        };
        ModuleManager.prototype.getGlobalAMDDefineFunc = function () {
            return this._defineFunc;
        };
        ModuleManager.prototype.getGlobalAMDRequireFunc = function () {
            return this._requireFunc;
        };
        ModuleManager._findRelevantLocationInStack = function (needle, stack) {
            var normalize = function (str) { return str.replace(/\\/g, '/'); };
            var normalizedPath = normalize(needle);
            var stackPieces = stack.split(/\n/);
            for (var i = 0; i < stackPieces.length; i++) {
                var m = stackPieces[i].match(/(.*):(\d+):(\d+)\)?$/);
                if (m) {
                    var stackPath = m[1];
                    var stackLine = m[2];
                    var stackColumn = m[3];
                    var trimPathOffset = Math.max(stackPath.lastIndexOf(' ') + 1, stackPath.lastIndexOf('(') + 1);
                    stackPath = stackPath.substr(trimPathOffset);
                    stackPath = normalize(stackPath);
                    if (stackPath === normalizedPath) {
                        var r = {
                            line: parseInt(stackLine, 10),
                            col: parseInt(stackColumn, 10)
                        };
                        if (r.line === 1) {
                            r.col -= '(function (require, define, __filename, __dirname) { '.length;
                        }
                        return r;
                    }
                }
            }
            throw new Error('Could not correlate define call site for needle ' + needle);
        };
        ModuleManager.prototype.getBuildInfo = function () {
            if (!this._config.isBuild()) {
                return null;
            }
            var result = [], resultLen = 0;
            for (var i = 0, len = this._modules2.length; i < len; i++) {
                var m = this._modules2[i];
                if (!m) {
                    continue;
                }
                var location_1 = this._buildInfoPath[m.id] || null;
                var defineStack = this._buildInfoDefineStack[m.id] || null;
                var dependencies = this._buildInfoDependencies[m.id];
                result[resultLen++] = {
                    id: m.strId,
                    path: location_1,
                    defineLocation: (location_1 && defineStack ? ModuleManager._findRelevantLocationInStack(location_1, defineStack) : null),
                    dependencies: dependencies,
                    shim: null,
                    exports: m.exports
                };
            }
            return result;
        };
        ModuleManager.prototype.getRecorder = function () {
            if (!this._recorder) {
                if (this._config.shouldRecordStats()) {
                    this._recorder = new AMDLoader.LoaderEventRecorder(this._loaderAvailableTimestamp);
                }
                else {
                    this._recorder = AMDLoader.NullLoaderEventRecorder.INSTANCE;
                }
            }
            return this._recorder;
        };
        ModuleManager.prototype.getLoaderEvents = function () {
            return this.getRecorder().getEvents();
        };
        /**
         * Defines an anonymous module (without an id). Its name will be resolved as we receive a callback from the scriptLoader.
         * @param dependecies @see defineModule
         * @param callback @see defineModule
         */
        ModuleManager.prototype.enqueueDefineAnonymousModule = function (dependencies, callback) {
            if (this._currentAnnonymousDefineCall !== null) {
                throw new Error('Can only have one anonymous define call per script file');
            }
            var stack = null;
            if (this._config.isBuild()) {
                stack = new Error('StackLocation').stack || null;
            }
            this._currentAnnonymousDefineCall = {
                stack: stack,
                dependencies: dependencies,
                callback: callback
            };
        };
        /**
         * Creates a module and stores it in _modules. The manager will immediately begin resolving its dependencies.
         * @param strModuleId An unique and absolute id of the module. This must not collide with another module's id
         * @param dependencies An array with the dependencies of the module. Special keys are: "require", "exports" and "module"
         * @param callback if callback is a function, it will be called with the resolved dependencies. if callback is an object, it will be considered as the exports of the module.
         */
        ModuleManager.prototype.defineModule = function (strModuleId, dependencies, callback, errorback, stack, moduleIdResolver) {
            var _this = this;
            if (moduleIdResolver === void 0) { moduleIdResolver = new ModuleIdResolver(strModuleId); }
            var moduleId = this._moduleIdProvider.getModuleId(strModuleId);
            if (this._modules2[moduleId]) {
                if (!this._config.isDuplicateMessageIgnoredFor(strModuleId)) {
                    console.warn('Duplicate definition of module \'' + strModuleId + '\'');
                }
                // Super important! Completely ignore duplicate module definition
                return;
            }
            var m = new Module(moduleId, strModuleId, this._normalizeDependencies(dependencies, moduleIdResolver), callback, errorback, moduleIdResolver);
            this._modules2[moduleId] = m;
            if (this._config.isBuild()) {
                this._buildInfoDefineStack[moduleId] = stack;
                this._buildInfoDependencies[moduleId] = (m.dependencies || []).map(function (dep) { return _this._moduleIdProvider.getStrModuleId(dep.id); });
            }
            // Resolving of dependencies is immediate (not in a timeout). If there's a need to support a packer that concatenates in an
            // unordered manner, in order to finish processing the file, execute the following method in a timeout
            this._resolve(m);
        };
        ModuleManager.prototype._normalizeDependency = function (dependency, moduleIdResolver) {
            if (dependency === 'exports') {
                return RegularDependency.EXPORTS;
            }
            if (dependency === 'module') {
                return RegularDependency.MODULE;
            }
            if (dependency === 'require') {
                return RegularDependency.REQUIRE;
            }
            // Normalize dependency and then request it from the manager
            var bangIndex = dependency.indexOf('!');
            if (bangIndex >= 0) {
                var strPluginId = moduleIdResolver.resolveModule(dependency.substr(0, bangIndex));
                var pluginParam = moduleIdResolver.resolveModule(dependency.substr(bangIndex + 1));
                var dependencyId = this._moduleIdProvider.getModuleId(strPluginId + '!' + pluginParam);
                var pluginId = this._moduleIdProvider.getModuleId(strPluginId);
                return new PluginDependency(dependencyId, pluginId, pluginParam);
            }
            return new RegularDependency(this._moduleIdProvider.getModuleId(moduleIdResolver.resolveModule(dependency)));
        };
        ModuleManager.prototype._normalizeDependencies = function (dependencies, moduleIdResolver) {
            var result = [], resultLen = 0;
            for (var i = 0, len = dependencies.length; i < len; i++) {
                result[resultLen++] = this._normalizeDependency(dependencies[i], moduleIdResolver);
            }
            return result;
        };
        ModuleManager.prototype._relativeRequire = function (moduleIdResolver, dependencies, callback, errorback) {
            if (typeof dependencies === 'string') {
                return this.synchronousRequire(dependencies, moduleIdResolver);
            }
            this.defineModule(AMDLoader.Utilities.generateAnonymousModule(), dependencies, callback, errorback, null, moduleIdResolver);
        };
        /**
         * Require synchronously a module by its absolute id. If the module is not loaded, an exception will be thrown.
         * @param id The unique and absolute id of the required module
         * @return The exports of module 'id'
         */
        ModuleManager.prototype.synchronousRequire = function (_strModuleId, moduleIdResolver) {
            if (moduleIdResolver === void 0) { moduleIdResolver = new ModuleIdResolver(_strModuleId); }
            var dependency = this._normalizeDependency(_strModuleId, moduleIdResolver);
            var m = this._modules2[dependency.id];
            if (!m) {
                throw new Error('Check dependency list! Synchronous require cannot resolve module \'' + _strModuleId + '\'. This is the first mention of this module!');
            }
            if (!m.isComplete()) {
                throw new Error('Check dependency list! Synchronous require cannot resolve module \'' + _strModuleId + '\'. This module has not been resolved completely yet.');
            }
            if (m.error) {
                throw m.error;
            }
            return m.exports;
        };
        ModuleManager.prototype.configure = function (params, shouldOverwrite) {
            var oldShouldRecordStats = this._config.shouldRecordStats();
            if (shouldOverwrite) {
                this._config = new AMDLoader.Configuration(this._env, params);
            }
            else {
                this._config = this._config.cloneAndMerge(params);
            }
            if (this._config.shouldRecordStats() && !oldShouldRecordStats) {
                this._recorder = null;
            }
        };
        ModuleManager.prototype.getConfig = function () {
            return this._config;
        };
        /**
         * Callback from the scriptLoader when a module has been loaded.
         * This means its code is available and has been executed.
         */
        ModuleManager.prototype._onLoad = function (moduleId) {
            if (this._currentAnnonymousDefineCall !== null) {
                var defineCall = this._currentAnnonymousDefineCall;
                this._currentAnnonymousDefineCall = null;
                // Hit an anonymous define call
                this.defineModule(this._moduleIdProvider.getStrModuleId(moduleId), defineCall.dependencies, defineCall.callback, null, defineCall.stack);
            }
        };
        ModuleManager.prototype._createLoadError = function (moduleId, _err) {
            var _this = this;
            var strModuleId = this._moduleIdProvider.getStrModuleId(moduleId);
            var neededBy = (this._inverseDependencies2[moduleId] || []).map(function (intModuleId) { return _this._moduleIdProvider.getStrModuleId(intModuleId); });
            var err = AMDLoader.ensureError(_err);
            err.phase = 'loading';
            err.moduleId = strModuleId;
            err.neededBy = neededBy;
            return err;
        };
        /**
         * Callback from the scriptLoader when a module hasn't been loaded.
         * This means that the script was not found (e.g. 404) or there was an error in the script.
         */
        ModuleManager.prototype._onLoadError = function (moduleId, err) {
            var error = this._createLoadError(moduleId, err);
            if (!this._modules2[moduleId]) {
                this._modules2[moduleId] = new Module(moduleId, this._moduleIdProvider.getStrModuleId(moduleId), [], function () { }, function () { }, null);
            }
            // Find any 'local' error handlers, walk the entire chain of inverse dependencies if necessary.
            var seenModuleId = [];
            for (var i = 0, len = this._moduleIdProvider.getMaxModuleId(); i < len; i++) {
                seenModuleId[i] = false;
            }
            var someoneNotified = false;
            var queue = [];
            queue.push(moduleId);
            seenModuleId[moduleId] = true;
            while (queue.length > 0) {
                var queueElement = queue.shift();
                var m = this._modules2[queueElement];
                if (m) {
                    someoneNotified = m.onDependencyError(error) || someoneNotified;
                }
                var inverseDeps = this._inverseDependencies2[queueElement];
                if (inverseDeps) {
                    for (var i = 0, len = inverseDeps.length; i < len; i++) {
                        var inverseDep = inverseDeps[i];
                        if (!seenModuleId[inverseDep]) {
                            queue.push(inverseDep);
                            seenModuleId[inverseDep] = true;
                        }
                    }
                }
            }
            if (!someoneNotified) {
                this._config.onError(error);
            }
        };
        /**
         * Walks (recursively) the dependencies of 'from' in search of 'to'.
         * Returns true if there is such a path or false otherwise.
         * @param from Module id to start at
         * @param to Module id to look for
         */
        ModuleManager.prototype._hasDependencyPath = function (fromId, toId) {
            var from = this._modules2[fromId];
            if (!from) {
                return false;
            }
            var inQueue = [];
            for (var i = 0, len = this._moduleIdProvider.getMaxModuleId(); i < len; i++) {
                inQueue[i] = false;
            }
            var queue = [];
            // Insert 'from' in queue
            queue.push(from);
            inQueue[fromId] = true;
            while (queue.length > 0) {
                // Pop first inserted element of queue
                var element = queue.shift();
                var dependencies = element.dependencies;
                if (dependencies) {
                    // Walk the element's dependencies
                    for (var i = 0, len = dependencies.length; i < len; i++) {
                        var dependency = dependencies[i];
                        if (dependency.id === toId) {
                            // There is a path to 'to'
                            return true;
                        }
                        var dependencyModule = this._modules2[dependency.id];
                        if (dependencyModule && !inQueue[dependency.id]) {
                            // Insert 'dependency' in queue
                            inQueue[dependency.id] = true;
                            queue.push(dependencyModule);
                        }
                    }
                }
            }
            // There is no path to 'to'
            return false;
        };
        /**
         * Walks (recursively) the dependencies of 'from' in search of 'to'.
         * Returns cycle as array.
         * @param from Module id to start at
         * @param to Module id to look for
         */
        ModuleManager.prototype._findCyclePath = function (fromId, toId, depth) {
            if (fromId === toId || depth === 50) {
                return [fromId];
            }
            var from = this._modules2[fromId];
            if (!from) {
                return null;
            }
            // Walk the element's dependencies
            var dependencies = from.dependencies;
            if (dependencies) {
                for (var i = 0, len = dependencies.length; i < len; i++) {
                    var path = this._findCyclePath(dependencies[i].id, toId, depth + 1);
                    if (path !== null) {
                        path.push(fromId);
                        return path;
                    }
                }
            }
            return null;
        };
        /**
         * Create the local 'require' that is passed into modules
         */
        ModuleManager.prototype._createRequire = function (moduleIdResolver) {
            var _this = this;
            var result = (function (dependencies, callback, errorback) {
                return _this._relativeRequire(moduleIdResolver, dependencies, callback, errorback);
            });
            result.toUrl = function (id) {
                return _this._config.requireToUrl(moduleIdResolver.resolveModule(id));
            };
            result.getStats = function () {
                return _this.getLoaderEvents();
            };
            result.__$__nodeRequire = AMDLoader.global.nodeRequire;
            return result;
        };
        ModuleManager.prototype._loadModule = function (moduleId) {
            var _this = this;
            if (this._modules2[moduleId] || this._knownModules2[moduleId]) {
                // known module
                return;
            }
            this._knownModules2[moduleId] = true;
            var strModuleId = this._moduleIdProvider.getStrModuleId(moduleId);
            var paths = this._config.moduleIdToPaths(strModuleId);
            var scopedPackageRegex = /^@[^\/]+\/[^\/]+$/; // matches @scope/package-name
            if (this._env.isNode && (strModuleId.indexOf('/') === -1 || scopedPackageRegex.test(strModuleId))) {
                paths.push('node|' + strModuleId);
            }
            var lastPathIndex = -1;
            var loadNextPath = function (err) {
                lastPathIndex++;
                if (lastPathIndex >= paths.length) {
                    // No more paths to try
                    _this._onLoadError(moduleId, err);
                }
                else {
                    var currentPath_1 = paths[lastPathIndex];
                    var recorder_1 = _this.getRecorder();
                    if (_this._config.isBuild() && currentPath_1 === 'empty:') {
                        _this._buildInfoPath[moduleId] = currentPath_1;
                        _this.defineModule(_this._moduleIdProvider.getStrModuleId(moduleId), [], null, null, null);
                        _this._onLoad(moduleId);
                        return;
                    }
                    recorder_1.record(10 /* BeginLoadingScript */, currentPath_1);
                    _this._scriptLoader.load(_this, currentPath_1, function () {
                        if (_this._config.isBuild()) {
                            _this._buildInfoPath[moduleId] = currentPath_1;
                        }
                        recorder_1.record(11 /* EndLoadingScriptOK */, currentPath_1);
                        _this._onLoad(moduleId);
                    }, function (err) {
                        recorder_1.record(12 /* EndLoadingScriptError */, currentPath_1);
                        loadNextPath(err);
                    });
                }
            };
            loadNextPath(null);
        };
        /**
         * Resolve a plugin dependency with the plugin loaded & complete
         * @param module The module that has this dependency
         * @param pluginDependency The semi-normalized dependency that appears in the module. e.g. 'vs/css!./mycssfile'. Only the plugin part (before !) is normalized
         * @param plugin The plugin (what the plugin exports)
         */
        ModuleManager.prototype._loadPluginDependency = function (plugin, pluginDependency) {
            var _this = this;
            if (this._modules2[pluginDependency.id] || this._knownModules2[pluginDependency.id]) {
                // known module
                return;
            }
            this._knownModules2[pluginDependency.id] = true;
            // Delegate the loading of the resource to the plugin
            var load = (function (value) {
                _this.defineModule(_this._moduleIdProvider.getStrModuleId(pluginDependency.id), [], value, null, null);
            });
            load.error = function (err) {
                _this._config.onError(_this._createLoadError(pluginDependency.id, err));
            };
            plugin.load(pluginDependency.pluginParam, this._createRequire(ModuleIdResolver.ROOT), load, this._config.getOptionsLiteral());
        };
        /**
         * Examine the dependencies of module 'module' and resolve them as needed.
         */
        ModuleManager.prototype._resolve = function (module) {
            var _this = this;
            var dependencies = module.dependencies;
            if (dependencies) {
                for (var i = 0, len = dependencies.length; i < len; i++) {
                    var dependency = dependencies[i];
                    if (dependency === RegularDependency.EXPORTS) {
                        module.exportsPassedIn = true;
                        module.unresolvedDependenciesCount--;
                        continue;
                    }
                    if (dependency === RegularDependency.MODULE) {
                        module.unresolvedDependenciesCount--;
                        continue;
                    }
                    if (dependency === RegularDependency.REQUIRE) {
                        module.unresolvedDependenciesCount--;
                        continue;
                    }
                    var dependencyModule = this._modules2[dependency.id];
                    if (dependencyModule && dependencyModule.isComplete()) {
                        if (dependencyModule.error) {
                            module.onDependencyError(dependencyModule.error);
                            return;
                        }
                        module.unresolvedDependenciesCount--;
                        continue;
                    }
                    if (this._hasDependencyPath(dependency.id, module.id)) {
                        console.warn('There is a dependency cycle between \'' + this._moduleIdProvider.getStrModuleId(dependency.id) + '\' and \'' + this._moduleIdProvider.getStrModuleId(module.id) + '\'. The cyclic path follows:');
                        var cyclePath = this._findCyclePath(dependency.id, module.id, 0) || [];
                        cyclePath.reverse();
                        cyclePath.push(dependency.id);
                        console.warn(cyclePath.map(function (id) { return _this._moduleIdProvider.getStrModuleId(id); }).join(' => \n'));
                        // Break the cycle
                        module.unresolvedDependenciesCount--;
                        continue;
                    }
                    // record inverse dependency
                    this._inverseDependencies2[dependency.id] = this._inverseDependencies2[dependency.id] || [];
                    this._inverseDependencies2[dependency.id].push(module.id);
                    if (dependency instanceof PluginDependency) {
                        var plugin = this._modules2[dependency.pluginId];
                        if (plugin && plugin.isComplete()) {
                            this._loadPluginDependency(plugin.exports, dependency);
                            continue;
                        }
                        // Record dependency for when the plugin gets loaded
                        var inversePluginDeps = this._inversePluginDependencies2.get(dependency.pluginId);
                        if (!inversePluginDeps) {
                            inversePluginDeps = [];
                            this._inversePluginDependencies2.set(dependency.pluginId, inversePluginDeps);
                        }
                        inversePluginDeps.push(dependency);
                        this._loadModule(dependency.pluginId);
                        continue;
                    }
                    this._loadModule(dependency.id);
                }
            }
            if (module.unresolvedDependenciesCount === 0) {
                this._onModuleComplete(module);
            }
        };
        ModuleManager.prototype._onModuleComplete = function (module) {
            var _this = this;
            var recorder = this.getRecorder();
            if (module.isComplete()) {
                // already done
                return;
            }
            var dependencies = module.dependencies;
            var dependenciesValues = [];
            if (dependencies) {
                for (var i = 0, len = dependencies.length; i < len; i++) {
                    var dependency = dependencies[i];
                    if (dependency === RegularDependency.EXPORTS) {
                        dependenciesValues[i] = module.exports;
                        continue;
                    }
                    if (dependency === RegularDependency.MODULE) {
                        dependenciesValues[i] = {
                            id: module.strId,
                            config: function () {
                                return _this._config.getConfigForModule(module.strId);
                            }
                        };
                        continue;
                    }
                    if (dependency === RegularDependency.REQUIRE) {
                        dependenciesValues[i] = this._createRequire(module.moduleIdResolver);
                        continue;
                    }
                    var dependencyModule = this._modules2[dependency.id];
                    if (dependencyModule) {
                        dependenciesValues[i] = dependencyModule.exports;
                        continue;
                    }
                    dependenciesValues[i] = null;
                }
            }
            module.complete(recorder, this._config, dependenciesValues);
            // Fetch and clear inverse dependencies
            var inverseDeps = this._inverseDependencies2[module.id];
            this._inverseDependencies2[module.id] = null;
            if (inverseDeps) {
                // Resolve one inverse dependency at a time, always
                // on the lookout for a completed module.
                for (var i = 0, len = inverseDeps.length; i < len; i++) {
                    var inverseDependencyId = inverseDeps[i];
                    var inverseDependency = this._modules2[inverseDependencyId];
                    inverseDependency.unresolvedDependenciesCount--;
                    if (inverseDependency.unresolvedDependenciesCount === 0) {
                        this._onModuleComplete(inverseDependency);
                    }
                }
            }
            var inversePluginDeps = this._inversePluginDependencies2.get(module.id);
            if (inversePluginDeps) {
                // This module is used as a plugin at least once
                // Fetch and clear these inverse plugin dependencies
                this._inversePluginDependencies2.delete(module.id);
                // Resolve plugin dependencies one at a time
                for (var i = 0, len = inversePluginDeps.length; i < len; i++) {
                    this._loadPluginDependency(module.exports, inversePluginDeps[i]);
                }
            }
        };
        return ModuleManager;
    }());
    AMDLoader.ModuleManager = ModuleManager;
})(AMDLoader || (AMDLoader = {}));
var define;
var AMDLoader;
(function (AMDLoader) {
    var env = new AMDLoader.Environment();
    var moduleManager = null;
    var DefineFunc = function (id, dependencies, callback) {
        if (typeof id !== 'string') {
            callback = dependencies;
            dependencies = id;
            id = null;
        }
        if (typeof dependencies !== 'object' || !Array.isArray(dependencies)) {
            callback = dependencies;
            dependencies = null;
        }
        if (!dependencies) {
            dependencies = ['require', 'exports', 'module'];
        }
        if (id) {
            moduleManager.defineModule(id, dependencies, callback, null, null);
        }
        else {
            moduleManager.enqueueDefineAnonymousModule(dependencies, callback);
        }
    };
    DefineFunc.amd = {
        jQuery: true
    };
    var _requireFunc_config = function (params, shouldOverwrite) {
        if (shouldOverwrite === void 0) { shouldOverwrite = false; }
        moduleManager.configure(params, shouldOverwrite);
    };
    var RequireFunc = function () {
        if (arguments.length === 1) {
            if ((arguments[0] instanceof Object) && !Array.isArray(arguments[0])) {
                _requireFunc_config(arguments[0]);
                return;
            }
            if (typeof arguments[0] === 'string') {
                return moduleManager.synchronousRequire(arguments[0]);
            }
        }
        if (arguments.length === 2 || arguments.length === 3) {
            if (Array.isArray(arguments[0])) {
                moduleManager.defineModule(AMDLoader.Utilities.generateAnonymousModule(), arguments[0], arguments[1], arguments[2], null);
                return;
            }
        }
        throw new Error('Unrecognized require call');
    };
    RequireFunc.config = _requireFunc_config;
    RequireFunc.getConfig = function () {
        return moduleManager.getConfig().getOptionsLiteral();
    };
    RequireFunc.reset = function () {
        moduleManager = moduleManager.reset();
    };
    RequireFunc.getBuildInfo = function () {
        return moduleManager.getBuildInfo();
    };
    RequireFunc.getStats = function () {
        return moduleManager.getLoaderEvents();
    };
    RequireFunc.define = function () {
        return DefineFunc.apply(null, arguments);
    };
    function init() {
        if (typeof AMDLoader.global.require !== 'undefined' || typeof require !== 'undefined') {
            var _nodeRequire_1 = (AMDLoader.global.require || require);
            if (typeof _nodeRequire_1 === 'function' && typeof _nodeRequire_1.resolve === 'function') {
                // re-expose node's require function
                var nodeRequire = function (what) {
                    moduleManager.getRecorder().record(33 /* NodeBeginNativeRequire */, what);
                    try {
                        return _nodeRequire_1(what);
                    }
                    finally {
                        moduleManager.getRecorder().record(34 /* NodeEndNativeRequire */, what);
                    }
                };
                AMDLoader.global.nodeRequire = nodeRequire;
                RequireFunc.nodeRequire = nodeRequire;
                RequireFunc.__$__nodeRequire = nodeRequire;
            }
        }
        if (env.isNode && !env.isElectronRenderer) {
            module.exports = RequireFunc;
            require = RequireFunc;
        }
        else {
            if (!env.isElectronRenderer) {
                AMDLoader.global.define = DefineFunc;
            }
            AMDLoader.global.require = RequireFunc;
        }
    }
    AMDLoader.init = init;
    if (typeof AMDLoader.global.define !== 'function' || !AMDLoader.global.define.amd) {
        moduleManager = new AMDLoader.ModuleManager(env, AMDLoader.createScriptLoader(env), DefineFunc, RequireFunc, AMDLoader.Utilities.getHighPerformanceTimestamp());
        // The global variable require can configure the loader
        if (typeof AMDLoader.global.require !== 'undefined' && typeof AMDLoader.global.require !== 'function') {
            RequireFunc.config(AMDLoader.global.require);
        }
        // This define is for the local closure defined in node in the case that the loader is concatenated
        define = function () {
            return DefineFunc.apply(null, arguments);
        };
        define.amd = DefineFunc.amd;
        if (typeof doNotInitLoader === 'undefined') {
            init();
        }
    }
})(AMDLoader || (AMDLoader = {}));

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[14/*vs/base/common/diff/diffChange*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * Represents information about a specific difference between two sequences.
     */
    var DiffChange = /** @class */ (function () {
        /**
         * Constructs a new DiffChange with the given sequence information
         * and content.
         */
        function DiffChange(originalStart, originalLength, modifiedStart, modifiedLength) {
            //Debug.Assert(originalLength > 0 || modifiedLength > 0, "originalLength and modifiedLength cannot both be <= 0");
            this.originalStart = originalStart;
            this.originalLength = originalLength;
            this.modifiedStart = modifiedStart;
            this.modifiedLength = modifiedLength;
        }
        /**
         * The end point (exclusive) of the change in the original sequence.
         */
        DiffChange.prototype.getOriginalEnd = function () {
            return this.originalStart + this.originalLength;
        };
        /**
         * The end point (exclusive) of the change in the modified sequence.
         */
        DiffChange.prototype.getModifiedEnd = function () {
            return this.modifiedStart + this.modifiedLength;
        };
        return DiffChange;
    }());
    exports.DiffChange = DiffChange;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
define(__m[3/*vs/base/common/errors*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    // Avoid circular dependency on EventEmitter by implementing a subset of the interface.
    var ErrorHandler = /** @class */ (function () {
        function ErrorHandler() {
            this.listeners = [];
            this.unexpectedErrorHandler = function (e) {
                setTimeout(function () {
                    if (e.stack) {
                        throw new Error(e.message + '\n\n' + e.stack);
                    }
                    throw e;
                }, 0);
            };
        }
        ErrorHandler.prototype.addListener = function (listener) {
            var _this = this;
            this.listeners.push(listener);
            return function () {
                _this._removeListener(listener);
            };
        };
        ErrorHandler.prototype.emit = function (e) {
            this.listeners.forEach(function (listener) {
                listener(e);
            });
        };
        ErrorHandler.prototype._removeListener = function (listener) {
            this.listeners.splice(this.listeners.indexOf(listener), 1);
        };
        ErrorHandler.prototype.setUnexpectedErrorHandler = function (newUnexpectedErrorHandler) {
            this.unexpectedErrorHandler = newUnexpectedErrorHandler;
        };
        ErrorHandler.prototype.getUnexpectedErrorHandler = function () {
            return this.unexpectedErrorHandler;
        };
        ErrorHandler.prototype.onUnexpectedError = function (e) {
            this.unexpectedErrorHandler(e);
            this.emit(e);
        };
        // For external errors, we don't want the listeners to be called
        ErrorHandler.prototype.onUnexpectedExternalError = function (e) {
            this.unexpectedErrorHandler(e);
        };
        return ErrorHandler;
    }());
    exports.ErrorHandler = ErrorHandler;
    exports.errorHandler = new ErrorHandler();
    function setUnexpectedErrorHandler(newUnexpectedErrorHandler) {
        exports.errorHandler.setUnexpectedErrorHandler(newUnexpectedErrorHandler);
    }
    exports.setUnexpectedErrorHandler = setUnexpectedErrorHandler;
    function onUnexpectedError(e) {
        // ignore errors from cancelled promises
        if (!isPromiseCanceledError(e)) {
            exports.errorHandler.onUnexpectedError(e);
        }
        return undefined;
    }
    exports.onUnexpectedError = onUnexpectedError;
    function onUnexpectedExternalError(e) {
        // ignore errors from cancelled promises
        if (!isPromiseCanceledError(e)) {
            exports.errorHandler.onUnexpectedExternalError(e);
        }
        return undefined;
    }
    exports.onUnexpectedExternalError = onUnexpectedExternalError;
    function transformErrorForSerialization(error) {
        if (error instanceof Error) {
            var name_1 = error.name, message = error.message;
            var stack = error.stacktrace || error.stack;
            return {
                $isError: true,
                name: name_1,
                message: message,
                stack: stack
            };
        }
        // return as is
        return error;
    }
    exports.transformErrorForSerialization = transformErrorForSerialization;
    var canceledName = 'Canceled';
    /**
     * Checks if the given error is a promise in canceled state
     */
    function isPromiseCanceledError(error) {
        return error instanceof Error && error.name === canceledName && error.message === canceledName;
    }
    exports.isPromiseCanceledError = isPromiseCanceledError;
    /**
     * Returns an error that signals cancellation.
     */
    function canceled() {
        var error = new Error(canceledName);
        error.name = error.message;
        return error;
    }
    exports.canceled = canceled;
    function illegalArgument(name) {
        if (name) {
            return new Error("Illegal argument: " + name);
        }
        else {
            return new Error('Illegal argument');
        }
    }
    exports.illegalArgument = illegalArgument;
    function illegalState(name) {
        if (name) {
            return new Error("Illegal state: " + name);
        }
        else {
            return new Error('Illegal state');
        }
    }
    exports.illegalState = illegalState;
    function readonly(name) {
        return name
            ? new Error("readonly property '" + name + " cannot be changed'")
            : new Error('readonly property cannot be changed');
    }
    exports.readonly = readonly;
    function disposed(what) {
        var result = new Error(what + " has been disposed");
        result.name = 'DISPOSED';
        return result;
    }
    exports.disposed = disposed;
    function getErrorMessage(err) {
        if (!err) {
            return 'Error';
        }
        if (err.message) {
            return err.message;
        }
        if (err.stack) {
            return err.stack.split('\n')[0];
        }
        return String(err);
    }
    exports.getErrorMessage = getErrorMessage;
    var NotImplementedError = /** @class */ (function (_super) {
        __extends(NotImplementedError, _super);
        function NotImplementedError(message) {
            var _this = _super.call(this, 'NotImplemented') || this;
            if (message) {
                _this.message = message;
            }
            return _this;
        }
        return NotImplementedError;
    }(Error));
    exports.NotImplementedError = NotImplementedError;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
define(__m[21/*vs/base/common/arrays*/], __M([1/*require*/,0/*exports*/,3/*vs/base/common/errors*/]), function (require, exports, errors_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * Returns the last element of an array.
     * @param array The array.
     * @param n Which element from the end (default is zero).
     */
    function tail(array, n) {
        if (n === void 0) { n = 0; }
        return array[array.length - (1 + n)];
    }
    exports.tail = tail;
    function tail2(arr) {
        if (arr.length === 0) {
            throw new Error('Invalid tail call');
        }
        return [arr.slice(0, arr.length - 1), arr[arr.length - 1]];
    }
    exports.tail2 = tail2;
    function equals(one, other, itemEquals) {
        if (itemEquals === void 0) { itemEquals = function (a, b) { return a === b; }; }
        if (one === other) {
            return true;
        }
        if (!one || !other) {
            return false;
        }
        if (one.length !== other.length) {
            return false;
        }
        for (var i = 0, len = one.length; i < len; i++) {
            if (!itemEquals(one[i], other[i])) {
                return false;
            }
        }
        return true;
    }
    exports.equals = equals;
    function binarySearch(array, key, comparator) {
        var low = 0, high = array.length - 1;
        while (low <= high) {
            var mid = ((low + high) / 2) | 0;
            var comp = comparator(array[mid], key);
            if (comp < 0) {
                low = mid + 1;
            }
            else if (comp > 0) {
                high = mid - 1;
            }
            else {
                return mid;
            }
        }
        return -(low + 1);
    }
    exports.binarySearch = binarySearch;
    /**
     * Takes a sorted array and a function p. The array is sorted in such a way that all elements where p(x) is false
     * are located before all elements where p(x) is true.
     * @returns the least x for which p(x) is true or array.length if no element fullfills the given function.
     */
    function findFirstInSorted(array, p) {
        var low = 0, high = array.length;
        if (high === 0) {
            return 0; // no children
        }
        while (low < high) {
            var mid = Math.floor((low + high) / 2);
            if (p(array[mid])) {
                high = mid;
            }
            else {
                low = mid + 1;
            }
        }
        return low;
    }
    exports.findFirstInSorted = findFirstInSorted;
    /**
     * Like `Array#sort` but always stable. Usually runs a little slower `than Array#sort`
     * so only use this when actually needing stable sort.
     */
    function mergeSort(data, compare) {
        _sort(data, compare, 0, data.length - 1, []);
        return data;
    }
    exports.mergeSort = mergeSort;
    function _merge(a, compare, lo, mid, hi, aux) {
        var leftIdx = lo, rightIdx = mid + 1;
        for (var i = lo; i <= hi; i++) {
            aux[i] = a[i];
        }
        for (var i = lo; i <= hi; i++) {
            if (leftIdx > mid) {
                // left side consumed
                a[i] = aux[rightIdx++];
            }
            else if (rightIdx > hi) {
                // right side consumed
                a[i] = aux[leftIdx++];
            }
            else if (compare(aux[rightIdx], aux[leftIdx]) < 0) {
                // right element is less -> comes first
                a[i] = aux[rightIdx++];
            }
            else {
                // left element comes first (less or equal)
                a[i] = aux[leftIdx++];
            }
        }
    }
    function _sort(a, compare, lo, hi, aux) {
        if (hi <= lo) {
            return;
        }
        var mid = lo + ((hi - lo) / 2) | 0;
        _sort(a, compare, lo, mid, aux);
        _sort(a, compare, mid + 1, hi, aux);
        if (compare(a[mid], a[mid + 1]) <= 0) {
            // left and right are sorted and if the last-left element is less
            // or equals than the first-right element there is nothing else
            // to do
            return;
        }
        _merge(a, compare, lo, mid, hi, aux);
    }
    function groupBy(data, compare) {
        var result = [];
        var currentGroup = undefined;
        for (var _i = 0, _a = mergeSort(data.slice(0), compare); _i < _a.length; _i++) {
            var element = _a[_i];
            if (!currentGroup || compare(currentGroup[0], element) !== 0) {
                currentGroup = [element];
                result.push(currentGroup);
            }
            else {
                currentGroup.push(element);
            }
        }
        return result;
    }
    exports.groupBy = groupBy;
    /**
     * Diffs two *sorted* arrays and computes the splices which apply the diff.
     */
    function sortedDiff(before, after, compare) {
        var result = [];
        function pushSplice(start, deleteCount, toInsert) {
            var _a;
            if (deleteCount === 0 && toInsert.length === 0) {
                return;
            }
            var latest = result[result.length - 1];
            if (latest && latest.start + latest.deleteCount === start) {
                latest.deleteCount += deleteCount;
                (_a = latest.toInsert).push.apply(_a, toInsert);
            }
            else {
                result.push({ start: start, deleteCount: deleteCount, toInsert: toInsert });
            }
        }
        var beforeIdx = 0;
        var afterIdx = 0;
        while (true) {
            if (beforeIdx === before.length) {
                pushSplice(beforeIdx, 0, after.slice(afterIdx));
                break;
            }
            if (afterIdx === after.length) {
                pushSplice(beforeIdx, before.length - beforeIdx, []);
                break;
            }
            var beforeElement = before[beforeIdx];
            var afterElement = after[afterIdx];
            var n = compare(beforeElement, afterElement);
            if (n === 0) {
                // equal
                beforeIdx += 1;
                afterIdx += 1;
            }
            else if (n < 0) {
                // beforeElement is smaller -> before element removed
                pushSplice(beforeIdx, 1, []);
                beforeIdx += 1;
            }
            else if (n > 0) {
                // beforeElement is greater -> after element added
                pushSplice(beforeIdx, 0, [afterElement]);
                afterIdx += 1;
            }
        }
        return result;
    }
    exports.sortedDiff = sortedDiff;
    /**
     * Takes two *sorted* arrays and computes their delta (removed, added elements).
     * Finishes in `Math.min(before.length, after.length)` steps.
     */
    function delta(before, after, compare) {
        var splices = sortedDiff(before, after, compare);
        var removed = [];
        var added = [];
        for (var _i = 0, splices_1 = splices; _i < splices_1.length; _i++) {
            var splice = splices_1[_i];
            removed.push.apply(removed, before.slice(splice.start, splice.start + splice.deleteCount));
            added.push.apply(added, splice.toInsert);
        }
        return { removed: removed, added: added };
    }
    exports.delta = delta;
    /**
     * Returns the top N elements from the array.
     *
     * Faster than sorting the entire array when the array is a lot larger than N.
     *
     * @param array The unsorted array.
     * @param compare A sort function for the elements.
     * @param n The number of elements to return.
     * @return The first n elemnts from array when sorted with compare.
     */
    function top(array, compare, n) {
        if (n === 0) {
            return [];
        }
        var result = array.slice(0, n).sort(compare);
        topStep(array, compare, result, n, array.length);
        return result;
    }
    exports.top = top;
    /**
     * Asynchronous variant of `top()` allowing for splitting up work in batches between which the event loop can run.
     *
     * Returns the top N elements from the array.
     *
     * Faster than sorting the entire array when the array is a lot larger than N.
     *
     * @param array The unsorted array.
     * @param compare A sort function for the elements.
     * @param n The number of elements to return.
     * @param batch The number of elements to examine before yielding to the event loop.
     * @return The first n elemnts from array when sorted with compare.
     */
    function topAsync(array, compare, n, batch, token) {
        var _this = this;
        if (n === 0) {
            return Promise.resolve([]);
        }
        return new Promise(function (resolve, reject) {
            (function () { return __awaiter(_this, void 0, void 0, function () {
                var o, result, i, m;
                return __generator(this, function (_a) {
                    switch (_a.label) {
                        case 0:
                            o = array.length;
                            result = array.slice(0, n).sort(compare);
                            i = n, m = Math.min(n + batch, o);
                            _a.label = 1;
                        case 1:
                            if (!(i < o)) return [3 /*break*/, 5];
                            if (!(i > n)) return [3 /*break*/, 3];
                            return [4 /*yield*/, new Promise(function (resolve) { return setTimeout(resolve); })];
                        case 2:
                            _a.sent(); // nextTick() would starve I/O.
                            _a.label = 3;
                        case 3:
                            if (token && token.isCancellationRequested) {
                                throw errors_1.canceled();
                            }
                            topStep(array, compare, result, i, m);
                            _a.label = 4;
                        case 4:
                            i = m, m = Math.min(m + batch, o);
                            return [3 /*break*/, 1];
                        case 5: return [2 /*return*/, result];
                    }
                });
            }); })()
                .then(resolve, reject);
        });
    }
    exports.topAsync = topAsync;
    function topStep(array, compare, result, i, m) {
        var _loop_1 = function (n) {
            var element = array[i];
            if (compare(element, result[n - 1]) < 0) {
                result.pop();
                var j = findFirstInSorted(result, function (e) { return compare(element, e) < 0; });
                result.splice(j, 0, element);
            }
        };
        for (var n = result.length; i < m; i++) {
            _loop_1(n);
        }
    }
    /**
     * @returns New array with all falsy values removed. The original array IS NOT modified.
     */
    function coalesce(array) {
        return array.filter(function (e) { return !!e; });
    }
    exports.coalesce = coalesce;
    /**
     * Remove all falsey values from `array`. The original array IS modified.
     */
    function coalesceInPlace(array) {
        var to = 0;
        for (var i = 0; i < array.length; i++) {
            if (!!array[i]) {
                array[to] = array[i];
                to += 1;
            }
        }
        array.length = to;
    }
    exports.coalesceInPlace = coalesceInPlace;
    /**
     * Moves the element in the array for the provided positions.
     */
    function move(array, from, to) {
        array.splice(to, 0, array.splice(from, 1)[0]);
    }
    exports.move = move;
    /**
     * @returns false if the provided object is an array and not empty.
     */
    function isFalsyOrEmpty(obj) {
        return !Array.isArray(obj) || obj.length === 0;
    }
    exports.isFalsyOrEmpty = isFalsyOrEmpty;
    function isNonEmptyArray(obj) {
        return Array.isArray(obj) && obj.length > 0;
    }
    exports.isNonEmptyArray = isNonEmptyArray;
    /**
     * Removes duplicates from the given array. The optional keyFn allows to specify
     * how elements are checked for equalness by returning a unique string for each.
     */
    function distinct(array, keyFn) {
        if (!keyFn) {
            return array.filter(function (element, position) {
                return array.indexOf(element) === position;
            });
        }
        var seen = Object.create(null);
        return array.filter(function (elem) {
            var key = keyFn(elem);
            if (seen[key]) {
                return false;
            }
            seen[key] = true;
            return true;
        });
    }
    exports.distinct = distinct;
    function distinctES6(array) {
        var seen = new Set();
        return array.filter(function (element) {
            if (seen.has(element)) {
                return false;
            }
            seen.add(element);
            return true;
        });
    }
    exports.distinctES6 = distinctES6;
    function fromSet(set) {
        var result = [];
        set.forEach(function (o) { return result.push(o); });
        return result;
    }
    exports.fromSet = fromSet;
    function uniqueFilter(keyFn) {
        var seen = Object.create(null);
        return function (element) {
            var key = keyFn(element);
            if (seen[key]) {
                return false;
            }
            seen[key] = true;
            return true;
        };
    }
    exports.uniqueFilter = uniqueFilter;
    function lastIndex(array, fn) {
        for (var i = array.length - 1; i >= 0; i--) {
            var element = array[i];
            if (fn(element)) {
                return i;
            }
        }
        return -1;
    }
    exports.lastIndex = lastIndex;
    function firstIndex(array, fn) {
        for (var i = 0; i < array.length; i++) {
            var element = array[i];
            if (fn(element)) {
                return i;
            }
        }
        return -1;
    }
    exports.firstIndex = firstIndex;
    function first(array, fn, notFoundValue) {
        if (notFoundValue === void 0) { notFoundValue = undefined; }
        var index = firstIndex(array, fn);
        return index < 0 ? notFoundValue : array[index];
    }
    exports.first = first;
    function firstOrDefault(array, notFoundValue) {
        return array.length > 0 ? array[0] : notFoundValue;
    }
    exports.firstOrDefault = firstOrDefault;
    function commonPrefixLength(one, other, equals) {
        if (equals === void 0) { equals = function (a, b) { return a === b; }; }
        var result = 0;
        for (var i = 0, len = Math.min(one.length, other.length); i < len && equals(one[i], other[i]); i++) {
            result++;
        }
        return result;
    }
    exports.commonPrefixLength = commonPrefixLength;
    function flatten(arr) {
        var _a;
        return (_a = []).concat.apply(_a, arr);
    }
    exports.flatten = flatten;
    function range(arg, to) {
        var from = typeof to === 'number' ? arg : 0;
        if (typeof to === 'number') {
            from = arg;
        }
        else {
            from = 0;
            to = arg;
        }
        var result = [];
        if (from <= to) {
            for (var i = from; i < to; i++) {
                result.push(i);
            }
        }
        else {
            for (var i = from; i > to; i--) {
                result.push(i);
            }
        }
        return result;
    }
    exports.range = range;
    function fill(num, value, arr) {
        if (arr === void 0) { arr = []; }
        for (var i = 0; i < num; i++) {
            arr[i] = value;
        }
        return arr;
    }
    exports.fill = fill;
    function index(array, indexer, merger) {
        if (merger === void 0) { merger = function (t) { return t; }; }
        return array.reduce(function (r, t) {
            var key = indexer(t);
            r[key] = merger(t, r[key]);
            return r;
        }, Object.create(null));
    }
    exports.index = index;
    /**
     * Inserts an element into an array. Returns a function which, when
     * called, will remove that element from the array.
     */
    function insert(array, element) {
        array.push(element);
        return function () {
            var index = array.indexOf(element);
            if (index > -1) {
                array.splice(index, 1);
            }
        };
    }
    exports.insert = insert;
    /**
     * Insert `insertArr` inside `target` at `insertIndex`.
     * Please don't touch unless you understand https://jsperf.com/inserting-an-array-within-an-array
     */
    function arrayInsert(target, insertIndex, insertArr) {
        var before = target.slice(0, insertIndex);
        var after = target.slice(insertIndex);
        return before.concat(insertArr, after);
    }
    exports.arrayInsert = arrayInsert;
    /**
     * Uses Fisher-Yates shuffle to shuffle the given array
     */
    function shuffle(array, _seed) {
        var rand;
        if (typeof _seed === 'number') {
            var seed_1 = _seed;
            // Seeded random number generator in JS. Modified from:
            // https://stackoverflow.com/questions/521295/seeding-the-random-number-generator-in-javascript
            rand = function () {
                var x = Math.sin(seed_1++) * 179426549; // throw away most significant digits and reduce any potential bias
                return x - Math.floor(x);
            };
        }
        else {
            rand = Math.random;
        }
        for (var i = array.length - 1; i > 0; i -= 1) {
            var j = Math.floor(rand() * (i + 1));
            var temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
    exports.shuffle = shuffle;
    /**
     * Pushes an element to the start of the array, if found.
     */
    function pushToStart(arr, value) {
        var index = arr.indexOf(value);
        if (index > -1) {
            arr.splice(index, 1);
            arr.unshift(value);
        }
    }
    exports.pushToStart = pushToStart;
    /**
     * Pushes an element to the end of the array, if found.
     */
    function pushToEnd(arr, value) {
        var index = arr.indexOf(value);
        if (index > -1) {
            arr.splice(index, 1);
            arr.push(value);
        }
    }
    exports.pushToEnd = pushToEnd;
    function find(arr, predicate) {
        for (var i = 0; i < arr.length; i++) {
            var element = arr[i];
            if (predicate(element, i, arr)) {
                return element;
            }
        }
        return undefined;
    }
    exports.find = find;
    function mapArrayOrNot(items, fn) {
        return Array.isArray(items) ?
            items.map(fn) :
            fn(items);
    }
    exports.mapArrayOrNot = mapArrayOrNot;
    function asArray(x) {
        return Array.isArray(x) ? x : [x];
    }
    exports.asArray = asArray;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[8/*vs/base/common/functional*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    function once(fn) {
        var _this = this;
        var didCall = false;
        var result;
        return function () {
            if (didCall) {
                return result;
            }
            didCall = true;
            result = fn.apply(_this, arguments);
            return result;
        };
    }
    exports.once = once;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[17/*vs/base/common/hash*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * Return a hash value for an object.
     */
    function hash(obj, hashVal) {
        if (hashVal === void 0) { hashVal = 0; }
        switch (typeof obj) {
            case 'object':
                if (obj === null) {
                    return numberHash(349, hashVal);
                }
                else if (Array.isArray(obj)) {
                    return arrayHash(obj, hashVal);
                }
                return objectHash(obj, hashVal);
            case 'string':
                return stringHash(obj, hashVal);
            case 'boolean':
                return booleanHash(obj, hashVal);
            case 'number':
                return numberHash(obj, hashVal);
            case 'undefined':
                return numberHash(0, 937);
            default:
                return numberHash(0, 617);
        }
    }
    exports.hash = hash;
    function numberHash(val, initialHashVal) {
        return (((initialHashVal << 5) - initialHashVal) + val) | 0; // hashVal * 31 + ch, keep as int32
    }
    function booleanHash(b, initialHashVal) {
        return numberHash(b ? 433 : 863, initialHashVal);
    }
    function stringHash(s, hashVal) {
        hashVal = numberHash(149417, hashVal);
        for (var i = 0, length_1 = s.length; i < length_1; i++) {
            hashVal = numberHash(s.charCodeAt(i), hashVal);
        }
        return hashVal;
    }
    exports.stringHash = stringHash;
    function arrayHash(arr, initialHashVal) {
        initialHashVal = numberHash(104579, initialHashVal);
        return arr.reduce(function (hashVal, item) { return hash(item, hashVal); }, initialHashVal);
    }
    function objectHash(obj, initialHashVal) {
        initialHashVal = numberHash(181387, initialHashVal);
        return Object.keys(obj).sort().reduce(function (hashVal, key) {
            hashVal = stringHash(key, hashVal);
            return hash(obj[key], hashVal);
        }, initialHashVal);
    }
    var Hasher = /** @class */ (function () {
        function Hasher() {
            this._value = 0;
        }
        Object.defineProperty(Hasher.prototype, "value", {
            get: function () {
                return this._value;
            },
            enumerable: true,
            configurable: true
        });
        Hasher.prototype.hash = function (obj) {
            this._value = hash(obj, this._value);
            return this._value;
        };
        return Hasher;
    }());
    exports.Hasher = Hasher;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[10/*vs/base/common/diff/diff*/], __M([1/*require*/,0/*exports*/,14/*vs/base/common/diff/diffChange*/,17/*vs/base/common/hash*/]), function (require, exports, diffChange_1, hash_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var StringDiffSequence = /** @class */ (function () {
        function StringDiffSequence(source) {
            this.source = source;
        }
        StringDiffSequence.prototype.getElements = function () {
            var source = this.source;
            var characters = new Int32Array(source.length);
            for (var i = 0, len = source.length; i < len; i++) {
                characters[i] = source.charCodeAt(i);
            }
            return characters;
        };
        return StringDiffSequence;
    }());
    exports.StringDiffSequence = StringDiffSequence;
    function stringDiff(original, modified, pretty) {
        return new LcsDiff(new StringDiffSequence(original), new StringDiffSequence(modified)).ComputeDiff(pretty).changes;
    }
    exports.stringDiff = stringDiff;
    //
    // The code below has been ported from a C# implementation in VS
    //
    var Debug = /** @class */ (function () {
        function Debug() {
        }
        Debug.Assert = function (condition, message) {
            if (!condition) {
                throw new Error(message);
            }
        };
        return Debug;
    }());
    exports.Debug = Debug;
    var MyArray = /** @class */ (function () {
        function MyArray() {
        }
        /**
         * Copies a range of elements from an Array starting at the specified source index and pastes
         * them to another Array starting at the specified destination index. The length and the indexes
         * are specified as 64-bit integers.
         * sourceArray:
         *		The Array that contains the data to copy.
         * sourceIndex:
         *		A 64-bit integer that represents the index in the sourceArray at which copying begins.
         * destinationArray:
         *		The Array that receives the data.
         * destinationIndex:
         *		A 64-bit integer that represents the index in the destinationArray at which storing begins.
         * length:
         *		A 64-bit integer that represents the number of elements to copy.
         */
        MyArray.Copy = function (sourceArray, sourceIndex, destinationArray, destinationIndex, length) {
            for (var i = 0; i < length; i++) {
                destinationArray[destinationIndex + i] = sourceArray[sourceIndex + i];
            }
        };
        MyArray.Copy2 = function (sourceArray, sourceIndex, destinationArray, destinationIndex, length) {
            for (var i = 0; i < length; i++) {
                destinationArray[destinationIndex + i] = sourceArray[sourceIndex + i];
            }
        };
        return MyArray;
    }());
    exports.MyArray = MyArray;
    /**
     * A utility class which helps to create the set of DiffChanges from
     * a difference operation. This class accepts original DiffElements and
     * modified DiffElements that are involved in a particular change. The
     * MarktNextChange() method can be called to mark the separation between
     * distinct changes. At the end, the Changes property can be called to retrieve
     * the constructed changes.
     */
    var DiffChangeHelper = /** @class */ (function () {
        /**
         * Constructs a new DiffChangeHelper for the given DiffSequences.
         */
        function DiffChangeHelper() {
            this.m_changes = [];
            this.m_originalStart = 1073741824 /* MAX_SAFE_SMALL_INTEGER */;
            this.m_modifiedStart = 1073741824 /* MAX_SAFE_SMALL_INTEGER */;
            this.m_originalCount = 0;
            this.m_modifiedCount = 0;
        }
        /**
         * Marks the beginning of the next change in the set of differences.
         */
        DiffChangeHelper.prototype.MarkNextChange = function () {
            // Only add to the list if there is something to add
            if (this.m_originalCount > 0 || this.m_modifiedCount > 0) {
                // Add the new change to our list
                this.m_changes.push(new diffChange_1.DiffChange(this.m_originalStart, this.m_originalCount, this.m_modifiedStart, this.m_modifiedCount));
            }
            // Reset for the next change
            this.m_originalCount = 0;
            this.m_modifiedCount = 0;
            this.m_originalStart = 1073741824 /* MAX_SAFE_SMALL_INTEGER */;
            this.m_modifiedStart = 1073741824 /* MAX_SAFE_SMALL_INTEGER */;
        };
        /**
         * Adds the original element at the given position to the elements
         * affected by the current change. The modified index gives context
         * to the change position with respect to the original sequence.
         * @param originalIndex The index of the original element to add.
         * @param modifiedIndex The index of the modified element that provides corresponding position in the modified sequence.
         */
        DiffChangeHelper.prototype.AddOriginalElement = function (originalIndex, modifiedIndex) {
            // The 'true' start index is the smallest of the ones we've seen
            this.m_originalStart = Math.min(this.m_originalStart, originalIndex);
            this.m_modifiedStart = Math.min(this.m_modifiedStart, modifiedIndex);
            this.m_originalCount++;
        };
        /**
         * Adds the modified element at the given position to the elements
         * affected by the current change. The original index gives context
         * to the change position with respect to the modified sequence.
         * @param originalIndex The index of the original element that provides corresponding position in the original sequence.
         * @param modifiedIndex The index of the modified element to add.
         */
        DiffChangeHelper.prototype.AddModifiedElement = function (originalIndex, modifiedIndex) {
            // The 'true' start index is the smallest of the ones we've seen
            this.m_originalStart = Math.min(this.m_originalStart, originalIndex);
            this.m_modifiedStart = Math.min(this.m_modifiedStart, modifiedIndex);
            this.m_modifiedCount++;
        };
        /**
         * Retrieves all of the changes marked by the class.
         */
        DiffChangeHelper.prototype.getChanges = function () {
            if (this.m_originalCount > 0 || this.m_modifiedCount > 0) {
                // Finish up on whatever is left
                this.MarkNextChange();
            }
            return this.m_changes;
        };
        /**
         * Retrieves all of the changes marked by the class in the reverse order
         */
        DiffChangeHelper.prototype.getReverseChanges = function () {
            if (this.m_originalCount > 0 || this.m_modifiedCount > 0) {
                // Finish up on whatever is left
                this.MarkNextChange();
            }
            this.m_changes.reverse();
            return this.m_changes;
        };
        return DiffChangeHelper;
    }());
    /**
     * An implementation of the difference algorithm described in
     * "An O(ND) Difference Algorithm and its variations" by Eugene W. Myers
     */
    var LcsDiff = /** @class */ (function () {
        /**
         * Constructs the DiffFinder
         */
        function LcsDiff(originalSequence, modifiedSequence, continueProcessingPredicate) {
            if (continueProcessingPredicate === void 0) { continueProcessingPredicate = null; }
            this.ContinueProcessingPredicate = continueProcessingPredicate;
            var _a = LcsDiff._getElements(originalSequence), originalStringElements = _a[0], originalElementsOrHash = _a[1], originalHasStrings = _a[2];
            var _b = LcsDiff._getElements(modifiedSequence), modifiedStringElements = _b[0], modifiedElementsOrHash = _b[1], modifiedHasStrings = _b[2];
            this._hasStrings = (originalHasStrings && modifiedHasStrings);
            this._originalStringElements = originalStringElements;
            this._originalElementsOrHash = originalElementsOrHash;
            this._modifiedStringElements = modifiedStringElements;
            this._modifiedElementsOrHash = modifiedElementsOrHash;
            this.m_forwardHistory = [];
            this.m_reverseHistory = [];
        }
        LcsDiff._isStringArray = function (arr) {
            return (arr.length > 0 && typeof arr[0] === 'string');
        };
        LcsDiff._getElements = function (sequence) {
            var elements = sequence.getElements();
            if (LcsDiff._isStringArray(elements)) {
                var hashes = new Int32Array(elements.length);
                for (var i = 0, len = elements.length; i < len; i++) {
                    hashes[i] = hash_1.stringHash(elements[i], 0);
                }
                return [elements, hashes, true];
            }
            if (elements instanceof Int32Array) {
                return [[], elements, false];
            }
            return [[], new Int32Array(elements), false];
        };
        LcsDiff.prototype.ElementsAreEqual = function (originalIndex, newIndex) {
            if (this._originalElementsOrHash[originalIndex] !== this._modifiedElementsOrHash[newIndex]) {
                return false;
            }
            return (this._hasStrings ? this._originalStringElements[originalIndex] === this._modifiedStringElements[newIndex] : true);
        };
        LcsDiff.prototype.OriginalElementsAreEqual = function (index1, index2) {
            if (this._originalElementsOrHash[index1] !== this._originalElementsOrHash[index2]) {
                return false;
            }
            return (this._hasStrings ? this._originalStringElements[index1] === this._originalStringElements[index2] : true);
        };
        LcsDiff.prototype.ModifiedElementsAreEqual = function (index1, index2) {
            if (this._modifiedElementsOrHash[index1] !== this._modifiedElementsOrHash[index2]) {
                return false;
            }
            return (this._hasStrings ? this._modifiedStringElements[index1] === this._modifiedStringElements[index2] : true);
        };
        LcsDiff.prototype.ComputeDiff = function (pretty) {
            return this._ComputeDiff(0, this._originalElementsOrHash.length - 1, 0, this._modifiedElementsOrHash.length - 1, pretty);
        };
        /**
         * Computes the differences between the original and modified input
         * sequences on the bounded range.
         * @returns An array of the differences between the two input sequences.
         */
        LcsDiff.prototype._ComputeDiff = function (originalStart, originalEnd, modifiedStart, modifiedEnd, pretty) {
            var quitEarlyArr = [false];
            var changes = this.ComputeDiffRecursive(originalStart, originalEnd, modifiedStart, modifiedEnd, quitEarlyArr);
            if (pretty) {
                // We have to clean up the computed diff to be more intuitive
                // but it turns out this cannot be done correctly until the entire set
                // of diffs have been computed
                changes = this.PrettifyChanges(changes);
            }
            return {
                quitEarly: quitEarlyArr[0],
                changes: changes
            };
        };
        /**
         * Private helper method which computes the differences on the bounded range
         * recursively.
         * @returns An array of the differences between the two input sequences.
         */
        LcsDiff.prototype.ComputeDiffRecursive = function (originalStart, originalEnd, modifiedStart, modifiedEnd, quitEarlyArr) {
            quitEarlyArr[0] = false;
            // Find the start of the differences
            while (originalStart <= originalEnd && modifiedStart <= modifiedEnd && this.ElementsAreEqual(originalStart, modifiedStart)) {
                originalStart++;
                modifiedStart++;
            }
            // Find the end of the differences
            while (originalEnd >= originalStart && modifiedEnd >= modifiedStart && this.ElementsAreEqual(originalEnd, modifiedEnd)) {
                originalEnd--;
                modifiedEnd--;
            }
            // In the special case where we either have all insertions or all deletions or the sequences are identical
            if (originalStart > originalEnd || modifiedStart > modifiedEnd) {
                var changes = void 0;
                if (modifiedStart <= modifiedEnd) {
                    Debug.Assert(originalStart === originalEnd + 1, 'originalStart should only be one more than originalEnd');
                    // All insertions
                    changes = [
                        new diffChange_1.DiffChange(originalStart, 0, modifiedStart, modifiedEnd - modifiedStart + 1)
                    ];
                }
                else if (originalStart <= originalEnd) {
                    Debug.Assert(modifiedStart === modifiedEnd + 1, 'modifiedStart should only be one more than modifiedEnd');
                    // All deletions
                    changes = [
                        new diffChange_1.DiffChange(originalStart, originalEnd - originalStart + 1, modifiedStart, 0)
                    ];
                }
                else {
                    Debug.Assert(originalStart === originalEnd + 1, 'originalStart should only be one more than originalEnd');
                    Debug.Assert(modifiedStart === modifiedEnd + 1, 'modifiedStart should only be one more than modifiedEnd');
                    // Identical sequences - No differences
                    changes = [];
                }
                return changes;
            }
            // This problem can be solved using the Divide-And-Conquer technique.
            var midOriginalArr = [0];
            var midModifiedArr = [0];
            var result = this.ComputeRecursionPoint(originalStart, originalEnd, modifiedStart, modifiedEnd, midOriginalArr, midModifiedArr, quitEarlyArr);
            var midOriginal = midOriginalArr[0];
            var midModified = midModifiedArr[0];
            if (result !== null) {
                // Result is not-null when there was enough memory to compute the changes while
                // searching for the recursion point
                return result;
            }
            else if (!quitEarlyArr[0]) {
                // We can break the problem down recursively by finding the changes in the
                // First Half:   (originalStart, modifiedStart) to (midOriginal, midModified)
                // Second Half:  (midOriginal + 1, minModified + 1) to (originalEnd, modifiedEnd)
                // NOTE: ComputeDiff() is inclusive, therefore the second range starts on the next point
                var leftChanges = this.ComputeDiffRecursive(originalStart, midOriginal, modifiedStart, midModified, quitEarlyArr);
                var rightChanges = [];
                if (!quitEarlyArr[0]) {
                    rightChanges = this.ComputeDiffRecursive(midOriginal + 1, originalEnd, midModified + 1, modifiedEnd, quitEarlyArr);
                }
                else {
                    // We did't have time to finish the first half, so we don't have time to compute this half.
                    // Consider the entire rest of the sequence different.
                    rightChanges = [
                        new diffChange_1.DiffChange(midOriginal + 1, originalEnd - (midOriginal + 1) + 1, midModified + 1, modifiedEnd - (midModified + 1) + 1)
                    ];
                }
                return this.ConcatenateChanges(leftChanges, rightChanges);
            }
            // If we hit here, we quit early, and so can't return anything meaningful
            return [
                new diffChange_1.DiffChange(originalStart, originalEnd - originalStart + 1, modifiedStart, modifiedEnd - modifiedStart + 1)
            ];
        };
        LcsDiff.prototype.WALKTRACE = function (diagonalForwardBase, diagonalForwardStart, diagonalForwardEnd, diagonalForwardOffset, diagonalReverseBase, diagonalReverseStart, diagonalReverseEnd, diagonalReverseOffset, forwardPoints, reversePoints, originalIndex, originalEnd, midOriginalArr, modifiedIndex, modifiedEnd, midModifiedArr, deltaIsEven, quitEarlyArr) {
            var forwardChanges = null;
            var reverseChanges = null;
            // First, walk backward through the forward diagonals history
            var changeHelper = new DiffChangeHelper();
            var diagonalMin = diagonalForwardStart;
            var diagonalMax = diagonalForwardEnd;
            var diagonalRelative = (midOriginalArr[0] - midModifiedArr[0]) - diagonalForwardOffset;
            var lastOriginalIndex = -1073741824 /* MIN_SAFE_SMALL_INTEGER */;
            var historyIndex = this.m_forwardHistory.length - 1;
            do {
                // Get the diagonal index from the relative diagonal number
                var diagonal = diagonalRelative + diagonalForwardBase;
                // Figure out where we came from
                if (diagonal === diagonalMin || (diagonal < diagonalMax && forwardPoints[diagonal - 1] < forwardPoints[diagonal + 1])) {
                    // Vertical line (the element is an insert)
                    originalIndex = forwardPoints[diagonal + 1];
                    modifiedIndex = originalIndex - diagonalRelative - diagonalForwardOffset;
                    if (originalIndex < lastOriginalIndex) {
                        changeHelper.MarkNextChange();
                    }
                    lastOriginalIndex = originalIndex;
                    changeHelper.AddModifiedElement(originalIndex + 1, modifiedIndex);
                    diagonalRelative = (diagonal + 1) - diagonalForwardBase; //Setup for the next iteration
                }
                else {
                    // Horizontal line (the element is a deletion)
                    originalIndex = forwardPoints[diagonal - 1] + 1;
                    modifiedIndex = originalIndex - diagonalRelative - diagonalForwardOffset;
                    if (originalIndex < lastOriginalIndex) {
                        changeHelper.MarkNextChange();
                    }
                    lastOriginalIndex = originalIndex - 1;
                    changeHelper.AddOriginalElement(originalIndex, modifiedIndex + 1);
                    diagonalRelative = (diagonal - 1) - diagonalForwardBase; //Setup for the next iteration
                }
                if (historyIndex >= 0) {
                    forwardPoints = this.m_forwardHistory[historyIndex];
                    diagonalForwardBase = forwardPoints[0]; //We stored this in the first spot
                    diagonalMin = 1;
                    diagonalMax = forwardPoints.length - 1;
                }
            } while (--historyIndex >= -1);
            // Ironically, we get the forward changes as the reverse of the
            // order we added them since we technically added them backwards
            forwardChanges = changeHelper.getReverseChanges();
            if (quitEarlyArr[0]) {
                // TODO: Calculate a partial from the reverse diagonals.
                //       For now, just assume everything after the midOriginal/midModified point is a diff
                var originalStartPoint = midOriginalArr[0] + 1;
                var modifiedStartPoint = midModifiedArr[0] + 1;
                if (forwardChanges !== null && forwardChanges.length > 0) {
                    var lastForwardChange = forwardChanges[forwardChanges.length - 1];
                    originalStartPoint = Math.max(originalStartPoint, lastForwardChange.getOriginalEnd());
                    modifiedStartPoint = Math.max(modifiedStartPoint, lastForwardChange.getModifiedEnd());
                }
                reverseChanges = [
                    new diffChange_1.DiffChange(originalStartPoint, originalEnd - originalStartPoint + 1, modifiedStartPoint, modifiedEnd - modifiedStartPoint + 1)
                ];
            }
            else {
                // Now walk backward through the reverse diagonals history
                changeHelper = new DiffChangeHelper();
                diagonalMin = diagonalReverseStart;
                diagonalMax = diagonalReverseEnd;
                diagonalRelative = (midOriginalArr[0] - midModifiedArr[0]) - diagonalReverseOffset;
                lastOriginalIndex = 1073741824 /* MAX_SAFE_SMALL_INTEGER */;
                historyIndex = (deltaIsEven) ? this.m_reverseHistory.length - 1 : this.m_reverseHistory.length - 2;
                do {
                    // Get the diagonal index from the relative diagonal number
                    var diagonal = diagonalRelative + diagonalReverseBase;
                    // Figure out where we came from
                    if (diagonal === diagonalMin || (diagonal < diagonalMax && reversePoints[diagonal - 1] >= reversePoints[diagonal + 1])) {
                        // Horizontal line (the element is a deletion))
                        originalIndex = reversePoints[diagonal + 1] - 1;
                        modifiedIndex = originalIndex - diagonalRelative - diagonalReverseOffset;
                        if (originalIndex > lastOriginalIndex) {
                            changeHelper.MarkNextChange();
                        }
                        lastOriginalIndex = originalIndex + 1;
                        changeHelper.AddOriginalElement(originalIndex + 1, modifiedIndex + 1);
                        diagonalRelative = (diagonal + 1) - diagonalReverseBase; //Setup for the next iteration
                    }
                    else {
                        // Vertical line (the element is an insertion)
                        originalIndex = reversePoints[diagonal - 1];
                        modifiedIndex = originalIndex - diagonalRelative - diagonalReverseOffset;
                        if (originalIndex > lastOriginalIndex) {
                            changeHelper.MarkNextChange();
                        }
                        lastOriginalIndex = originalIndex;
                        changeHelper.AddModifiedElement(originalIndex + 1, modifiedIndex + 1);
                        diagonalRelative = (diagonal - 1) - diagonalReverseBase; //Setup for the next iteration
                    }
                    if (historyIndex >= 0) {
                        reversePoints = this.m_reverseHistory[historyIndex];
                        diagonalReverseBase = reversePoints[0]; //We stored this in the first spot
                        diagonalMin = 1;
                        diagonalMax = reversePoints.length - 1;
                    }
                } while (--historyIndex >= -1);
                // There are cases where the reverse history will find diffs that
                // are correct, but not intuitive, so we need shift them.
                reverseChanges = changeHelper.getChanges();
            }
            return this.ConcatenateChanges(forwardChanges, reverseChanges);
        };
        /**
         * Given the range to compute the diff on, this method finds the point:
         * (midOriginal, midModified)
         * that exists in the middle of the LCS of the two sequences and
         * is the point at which the LCS problem may be broken down recursively.
         * This method will try to keep the LCS trace in memory. If the LCS recursion
         * point is calculated and the full trace is available in memory, then this method
         * will return the change list.
         * @param originalStart The start bound of the original sequence range
         * @param originalEnd The end bound of the original sequence range
         * @param modifiedStart The start bound of the modified sequence range
         * @param modifiedEnd The end bound of the modified sequence range
         * @param midOriginal The middle point of the original sequence range
         * @param midModified The middle point of the modified sequence range
         * @returns The diff changes, if available, otherwise null
         */
        LcsDiff.prototype.ComputeRecursionPoint = function (originalStart, originalEnd, modifiedStart, modifiedEnd, midOriginalArr, midModifiedArr, quitEarlyArr) {
            var originalIndex = 0, modifiedIndex = 0;
            var diagonalForwardStart = 0, diagonalForwardEnd = 0;
            var diagonalReverseStart = 0, diagonalReverseEnd = 0;
            // To traverse the edit graph and produce the proper LCS, our actual
            // start position is just outside the given boundary
            originalStart--;
            modifiedStart--;
            // We set these up to make the compiler happy, but they will
            // be replaced before we return with the actual recursion point
            midOriginalArr[0] = 0;
            midModifiedArr[0] = 0;
            // Clear out the history
            this.m_forwardHistory = [];
            this.m_reverseHistory = [];
            // Each cell in the two arrays corresponds to a diagonal in the edit graph.
            // The integer value in the cell represents the originalIndex of the furthest
            // reaching point found so far that ends in that diagonal.
            // The modifiedIndex can be computed mathematically from the originalIndex and the diagonal number.
            var maxDifferences = (originalEnd - originalStart) + (modifiedEnd - modifiedStart);
            var numDiagonals = maxDifferences + 1;
            var forwardPoints = new Int32Array(numDiagonals);
            var reversePoints = new Int32Array(numDiagonals);
            // diagonalForwardBase: Index into forwardPoints of the diagonal which passes through (originalStart, modifiedStart)
            // diagonalReverseBase: Index into reversePoints of the diagonal which passes through (originalEnd, modifiedEnd)
            var diagonalForwardBase = (modifiedEnd - modifiedStart);
            var diagonalReverseBase = (originalEnd - originalStart);
            // diagonalForwardOffset: Geometric offset which allows modifiedIndex to be computed from originalIndex and the
            //    diagonal number (relative to diagonalForwardBase)
            // diagonalReverseOffset: Geometric offset which allows modifiedIndex to be computed from originalIndex and the
            //    diagonal number (relative to diagonalReverseBase)
            var diagonalForwardOffset = (originalStart - modifiedStart);
            var diagonalReverseOffset = (originalEnd - modifiedEnd);
            // delta: The difference between the end diagonal and the start diagonal. This is used to relate diagonal numbers
            //   relative to the start diagonal with diagonal numbers relative to the end diagonal.
            // The Even/Oddn-ness of this delta is important for determining when we should check for overlap
            var delta = diagonalReverseBase - diagonalForwardBase;
            var deltaIsEven = (delta % 2 === 0);
            // Here we set up the start and end points as the furthest points found so far
            // in both the forward and reverse directions, respectively
            forwardPoints[diagonalForwardBase] = originalStart;
            reversePoints[diagonalReverseBase] = originalEnd;
            // Remember if we quit early, and thus need to do a best-effort result instead of a real result.
            quitEarlyArr[0] = false;
            // A couple of points:
            // --With this method, we iterate on the number of differences between the two sequences.
            //   The more differences there actually are, the longer this will take.
            // --Also, as the number of differences increases, we have to search on diagonals further
            //   away from the reference diagonal (which is diagonalForwardBase for forward, diagonalReverseBase for reverse).
            // --We extend on even diagonals (relative to the reference diagonal) only when numDifferences
            //   is even and odd diagonals only when numDifferences is odd.
            for (var numDifferences = 1; numDifferences <= (maxDifferences / 2) + 1; numDifferences++) {
                var furthestOriginalIndex = 0;
                var furthestModifiedIndex = 0;
                // Run the algorithm in the forward direction
                diagonalForwardStart = this.ClipDiagonalBound(diagonalForwardBase - numDifferences, numDifferences, diagonalForwardBase, numDiagonals);
                diagonalForwardEnd = this.ClipDiagonalBound(diagonalForwardBase + numDifferences, numDifferences, diagonalForwardBase, numDiagonals);
                for (var diagonal = diagonalForwardStart; diagonal <= diagonalForwardEnd; diagonal += 2) {
                    // STEP 1: We extend the furthest reaching point in the present diagonal
                    // by looking at the diagonals above and below and picking the one whose point
                    // is further away from the start point (originalStart, modifiedStart)
                    if (diagonal === diagonalForwardStart || (diagonal < diagonalForwardEnd && forwardPoints[diagonal - 1] < forwardPoints[diagonal + 1])) {
                        originalIndex = forwardPoints[diagonal + 1];
                    }
                    else {
                        originalIndex = forwardPoints[diagonal - 1] + 1;
                    }
                    modifiedIndex = originalIndex - (diagonal - diagonalForwardBase) - diagonalForwardOffset;
                    // Save the current originalIndex so we can test for false overlap in step 3
                    var tempOriginalIndex = originalIndex;
                    // STEP 2: We can continue to extend the furthest reaching point in the present diagonal
                    // so long as the elements are equal.
                    while (originalIndex < originalEnd && modifiedIndex < modifiedEnd && this.ElementsAreEqual(originalIndex + 1, modifiedIndex + 1)) {
                        originalIndex++;
                        modifiedIndex++;
                    }
                    forwardPoints[diagonal] = originalIndex;
                    if (originalIndex + modifiedIndex > furthestOriginalIndex + furthestModifiedIndex) {
                        furthestOriginalIndex = originalIndex;
                        furthestModifiedIndex = modifiedIndex;
                    }
                    // STEP 3: If delta is odd (overlap first happens on forward when delta is odd)
                    // and diagonal is in the range of reverse diagonals computed for numDifferences-1
                    // (the previous iteration; we haven't computed reverse diagonals for numDifferences yet)
                    // then check for overlap.
                    if (!deltaIsEven && Math.abs(diagonal - diagonalReverseBase) <= (numDifferences - 1)) {
                        if (originalIndex >= reversePoints[diagonal]) {
                            midOriginalArr[0] = originalIndex;
                            midModifiedArr[0] = modifiedIndex;
                            if (tempOriginalIndex <= reversePoints[diagonal] && 1447 /* MaxDifferencesHistory */ > 0 && numDifferences <= (1447 /* MaxDifferencesHistory */ + 1)) {
                                // BINGO! We overlapped, and we have the full trace in memory!
                                return this.WALKTRACE(diagonalForwardBase, diagonalForwardStart, diagonalForwardEnd, diagonalForwardOffset, diagonalReverseBase, diagonalReverseStart, diagonalReverseEnd, diagonalReverseOffset, forwardPoints, reversePoints, originalIndex, originalEnd, midOriginalArr, modifiedIndex, modifiedEnd, midModifiedArr, deltaIsEven, quitEarlyArr);
                            }
                            else {
                                // Either false overlap, or we didn't have enough memory for the full trace
                                // Just return the recursion point
                                return null;
                            }
                        }
                    }
                }
                // Check to see if we should be quitting early, before moving on to the next iteration.
                var matchLengthOfLongest = ((furthestOriginalIndex - originalStart) + (furthestModifiedIndex - modifiedStart) - numDifferences) / 2;
                if (this.ContinueProcessingPredicate !== null && !this.ContinueProcessingPredicate(furthestOriginalIndex, matchLengthOfLongest)) {
                    // We can't finish, so skip ahead to generating a result from what we have.
                    quitEarlyArr[0] = true;
                    // Use the furthest distance we got in the forward direction.
                    midOriginalArr[0] = furthestOriginalIndex;
                    midModifiedArr[0] = furthestModifiedIndex;
                    if (matchLengthOfLongest > 0 && 1447 /* MaxDifferencesHistory */ > 0 && numDifferences <= (1447 /* MaxDifferencesHistory */ + 1)) {
                        // Enough of the history is in memory to walk it backwards
                        return this.WALKTRACE(diagonalForwardBase, diagonalForwardStart, diagonalForwardEnd, diagonalForwardOffset, diagonalReverseBase, diagonalReverseStart, diagonalReverseEnd, diagonalReverseOffset, forwardPoints, reversePoints, originalIndex, originalEnd, midOriginalArr, modifiedIndex, modifiedEnd, midModifiedArr, deltaIsEven, quitEarlyArr);
                    }
                    else {
                        // We didn't actually remember enough of the history.
                        //Since we are quiting the diff early, we need to shift back the originalStart and modified start
                        //back into the boundary limits since we decremented their value above beyond the boundary limit.
                        originalStart++;
                        modifiedStart++;
                        return [
                            new diffChange_1.DiffChange(originalStart, originalEnd - originalStart + 1, modifiedStart, modifiedEnd - modifiedStart + 1)
                        ];
                    }
                }
                // Run the algorithm in the reverse direction
                diagonalReverseStart = this.ClipDiagonalBound(diagonalReverseBase - numDifferences, numDifferences, diagonalReverseBase, numDiagonals);
                diagonalReverseEnd = this.ClipDiagonalBound(diagonalReverseBase + numDifferences, numDifferences, diagonalReverseBase, numDiagonals);
                for (var diagonal = diagonalReverseStart; diagonal <= diagonalReverseEnd; diagonal += 2) {
                    // STEP 1: We extend the furthest reaching point in the present diagonal
                    // by looking at the diagonals above and below and picking the one whose point
                    // is further away from the start point (originalEnd, modifiedEnd)
                    if (diagonal === diagonalReverseStart || (diagonal < diagonalReverseEnd && reversePoints[diagonal - 1] >= reversePoints[diagonal + 1])) {
                        originalIndex = reversePoints[diagonal + 1] - 1;
                    }
                    else {
                        originalIndex = reversePoints[diagonal - 1];
                    }
                    modifiedIndex = originalIndex - (diagonal - diagonalReverseBase) - diagonalReverseOffset;
                    // Save the current originalIndex so we can test for false overlap
                    var tempOriginalIndex = originalIndex;
                    // STEP 2: We can continue to extend the furthest reaching point in the present diagonal
                    // as long as the elements are equal.
                    while (originalIndex > originalStart && modifiedIndex > modifiedStart && this.ElementsAreEqual(originalIndex, modifiedIndex)) {
                        originalIndex--;
                        modifiedIndex--;
                    }
                    reversePoints[diagonal] = originalIndex;
                    // STEP 4: If delta is even (overlap first happens on reverse when delta is even)
                    // and diagonal is in the range of forward diagonals computed for numDifferences
                    // then check for overlap.
                    if (deltaIsEven && Math.abs(diagonal - diagonalForwardBase) <= numDifferences) {
                        if (originalIndex <= forwardPoints[diagonal]) {
                            midOriginalArr[0] = originalIndex;
                            midModifiedArr[0] = modifiedIndex;
                            if (tempOriginalIndex >= forwardPoints[diagonal] && 1447 /* MaxDifferencesHistory */ > 0 && numDifferences <= (1447 /* MaxDifferencesHistory */ + 1)) {
                                // BINGO! We overlapped, and we have the full trace in memory!
                                return this.WALKTRACE(diagonalForwardBase, diagonalForwardStart, diagonalForwardEnd, diagonalForwardOffset, diagonalReverseBase, diagonalReverseStart, diagonalReverseEnd, diagonalReverseOffset, forwardPoints, reversePoints, originalIndex, originalEnd, midOriginalArr, modifiedIndex, modifiedEnd, midModifiedArr, deltaIsEven, quitEarlyArr);
                            }
                            else {
                                // Either false overlap, or we didn't have enough memory for the full trace
                                // Just return the recursion point
                                return null;
                            }
                        }
                    }
                }
                // Save current vectors to history before the next iteration
                if (numDifferences <= 1447 /* MaxDifferencesHistory */) {
                    // We are allocating space for one extra int, which we fill with
                    // the index of the diagonal base index
                    var temp = new Int32Array(diagonalForwardEnd - diagonalForwardStart + 2);
                    temp[0] = diagonalForwardBase - diagonalForwardStart + 1;
                    MyArray.Copy2(forwardPoints, diagonalForwardStart, temp, 1, diagonalForwardEnd - diagonalForwardStart + 1);
                    this.m_forwardHistory.push(temp);
                    temp = new Int32Array(diagonalReverseEnd - diagonalReverseStart + 2);
                    temp[0] = diagonalReverseBase - diagonalReverseStart + 1;
                    MyArray.Copy2(reversePoints, diagonalReverseStart, temp, 1, diagonalReverseEnd - diagonalReverseStart + 1);
                    this.m_reverseHistory.push(temp);
                }
            }
            // If we got here, then we have the full trace in history. We just have to convert it to a change list
            // NOTE: This part is a bit messy
            return this.WALKTRACE(diagonalForwardBase, diagonalForwardStart, diagonalForwardEnd, diagonalForwardOffset, diagonalReverseBase, diagonalReverseStart, diagonalReverseEnd, diagonalReverseOffset, forwardPoints, reversePoints, originalIndex, originalEnd, midOriginalArr, modifiedIndex, modifiedEnd, midModifiedArr, deltaIsEven, quitEarlyArr);
        };
        /**
         * Shifts the given changes to provide a more intuitive diff.
         * While the first element in a diff matches the first element after the diff,
         * we shift the diff down.
         *
         * @param changes The list of changes to shift
         * @returns The shifted changes
         */
        LcsDiff.prototype.PrettifyChanges = function (changes) {
            // Shift all the changes down first
            for (var i = 0; i < changes.length; i++) {
                var change = changes[i];
                var originalStop = (i < changes.length - 1) ? changes[i + 1].originalStart : this._originalElementsOrHash.length;
                var modifiedStop = (i < changes.length - 1) ? changes[i + 1].modifiedStart : this._modifiedElementsOrHash.length;
                var checkOriginal = change.originalLength > 0;
                var checkModified = change.modifiedLength > 0;
                while (change.originalStart + change.originalLength < originalStop &&
                    change.modifiedStart + change.modifiedLength < modifiedStop &&
                    (!checkOriginal || this.OriginalElementsAreEqual(change.originalStart, change.originalStart + change.originalLength)) &&
                    (!checkModified || this.ModifiedElementsAreEqual(change.modifiedStart, change.modifiedStart + change.modifiedLength))) {
                    change.originalStart++;
                    change.modifiedStart++;
                }
                var mergedChangeArr = [null];
                if (i < changes.length - 1 && this.ChangesOverlap(changes[i], changes[i + 1], mergedChangeArr)) {
                    changes[i] = mergedChangeArr[0];
                    changes.splice(i + 1, 1);
                    i--;
                    continue;
                }
            }
            // Shift changes back up until we hit empty or whitespace-only lines
            for (var i = changes.length - 1; i >= 0; i--) {
                var change = changes[i];
                var originalStop = 0;
                var modifiedStop = 0;
                if (i > 0) {
                    var prevChange = changes[i - 1];
                    if (prevChange.originalLength > 0) {
                        originalStop = prevChange.originalStart + prevChange.originalLength;
                    }
                    if (prevChange.modifiedLength > 0) {
                        modifiedStop = prevChange.modifiedStart + prevChange.modifiedLength;
                    }
                }
                var checkOriginal = change.originalLength > 0;
                var checkModified = change.modifiedLength > 0;
                var bestDelta = 0;
                var bestScore = this._boundaryScore(change.originalStart, change.originalLength, change.modifiedStart, change.modifiedLength);
                for (var delta = 1;; delta++) {
                    var originalStart = change.originalStart - delta;
                    var modifiedStart = change.modifiedStart - delta;
                    if (originalStart < originalStop || modifiedStart < modifiedStop) {
                        break;
                    }
                    if (checkOriginal && !this.OriginalElementsAreEqual(originalStart, originalStart + change.originalLength)) {
                        break;
                    }
                    if (checkModified && !this.ModifiedElementsAreEqual(modifiedStart, modifiedStart + change.modifiedLength)) {
                        break;
                    }
                    var score = this._boundaryScore(originalStart, change.originalLength, modifiedStart, change.modifiedLength);
                    if (score > bestScore) {
                        bestScore = score;
                        bestDelta = delta;
                    }
                }
                change.originalStart -= bestDelta;
                change.modifiedStart -= bestDelta;
            }
            return changes;
        };
        LcsDiff.prototype._OriginalIsBoundary = function (index) {
            if (index <= 0 || index >= this._originalElementsOrHash.length - 1) {
                return true;
            }
            return (this._hasStrings && /^\s*$/.test(this._originalStringElements[index]));
        };
        LcsDiff.prototype._OriginalRegionIsBoundary = function (originalStart, originalLength) {
            if (this._OriginalIsBoundary(originalStart) || this._OriginalIsBoundary(originalStart - 1)) {
                return true;
            }
            if (originalLength > 0) {
                var originalEnd = originalStart + originalLength;
                if (this._OriginalIsBoundary(originalEnd - 1) || this._OriginalIsBoundary(originalEnd)) {
                    return true;
                }
            }
            return false;
        };
        LcsDiff.prototype._ModifiedIsBoundary = function (index) {
            if (index <= 0 || index >= this._modifiedElementsOrHash.length - 1) {
                return true;
            }
            return (this._hasStrings && /^\s*$/.test(this._modifiedStringElements[index]));
        };
        LcsDiff.prototype._ModifiedRegionIsBoundary = function (modifiedStart, modifiedLength) {
            if (this._ModifiedIsBoundary(modifiedStart) || this._ModifiedIsBoundary(modifiedStart - 1)) {
                return true;
            }
            if (modifiedLength > 0) {
                var modifiedEnd = modifiedStart + modifiedLength;
                if (this._ModifiedIsBoundary(modifiedEnd - 1) || this._ModifiedIsBoundary(modifiedEnd)) {
                    return true;
                }
            }
            return false;
        };
        LcsDiff.prototype._boundaryScore = function (originalStart, originalLength, modifiedStart, modifiedLength) {
            var originalScore = (this._OriginalRegionIsBoundary(originalStart, originalLength) ? 1 : 0);
            var modifiedScore = (this._ModifiedRegionIsBoundary(modifiedStart, modifiedLength) ? 1 : 0);
            return (originalScore + modifiedScore);
        };
        /**
         * Concatenates the two input DiffChange lists and returns the resulting
         * list.
         * @param The left changes
         * @param The right changes
         * @returns The concatenated list
         */
        LcsDiff.prototype.ConcatenateChanges = function (left, right) {
            var mergedChangeArr = [];
            if (left.length === 0 || right.length === 0) {
                return (right.length > 0) ? right : left;
            }
            else if (this.ChangesOverlap(left[left.length - 1], right[0], mergedChangeArr)) {
                // Since we break the problem down recursively, it is possible that we
                // might recurse in the middle of a change thereby splitting it into
                // two changes. Here in the combining stage, we detect and fuse those
                // changes back together
                var result = new Array(left.length + right.length - 1);
                MyArray.Copy(left, 0, result, 0, left.length - 1);
                result[left.length - 1] = mergedChangeArr[0];
                MyArray.Copy(right, 1, result, left.length, right.length - 1);
                return result;
            }
            else {
                var result = new Array(left.length + right.length);
                MyArray.Copy(left, 0, result, 0, left.length);
                MyArray.Copy(right, 0, result, left.length, right.length);
                return result;
            }
        };
        /**
         * Returns true if the two changes overlap and can be merged into a single
         * change
         * @param left The left change
         * @param right The right change
         * @param mergedChange The merged change if the two overlap, null otherwise
         * @returns True if the two changes overlap
         */
        LcsDiff.prototype.ChangesOverlap = function (left, right, mergedChangeArr) {
            Debug.Assert(left.originalStart <= right.originalStart, 'Left change is not less than or equal to right change');
            Debug.Assert(left.modifiedStart <= right.modifiedStart, 'Left change is not less than or equal to right change');
            if (left.originalStart + left.originalLength >= right.originalStart || left.modifiedStart + left.modifiedLength >= right.modifiedStart) {
                var originalStart = left.originalStart;
                var originalLength = left.originalLength;
                var modifiedStart = left.modifiedStart;
                var modifiedLength = left.modifiedLength;
                if (left.originalStart + left.originalLength >= right.originalStart) {
                    originalLength = right.originalStart + right.originalLength - left.originalStart;
                }
                if (left.modifiedStart + left.modifiedLength >= right.modifiedStart) {
                    modifiedLength = right.modifiedStart + right.modifiedLength - left.modifiedStart;
                }
                mergedChangeArr[0] = new diffChange_1.DiffChange(originalStart, originalLength, modifiedStart, modifiedLength);
                return true;
            }
            else {
                mergedChangeArr[0] = null;
                return false;
            }
        };
        /**
         * Helper method used to clip a diagonal index to the range of valid
         * diagonals. This also decides whether or not the diagonal index,
         * if it exceeds the boundary, should be clipped to the boundary or clipped
         * one inside the boundary depending on the Even/Odd status of the boundary
         * and numDifferences.
         * @param diagonal The index of the diagonal to clip.
         * @param numDifferences The current number of differences being iterated upon.
         * @param diagonalBaseIndex The base reference diagonal.
         * @param numDiagonals The total number of diagonals.
         * @returns The clipped diagonal index.
         */
        LcsDiff.prototype.ClipDiagonalBound = function (diagonal, numDifferences, diagonalBaseIndex, numDiagonals) {
            if (diagonal >= 0 && diagonal < numDiagonals) {
                // Nothing to clip, its in range
                return diagonal;
            }
            // diagonalsBelow: The number of diagonals below the reference diagonal
            // diagonalsAbove: The number of diagonals above the reference diagonal
            var diagonalsBelow = diagonalBaseIndex;
            var diagonalsAbove = numDiagonals - diagonalBaseIndex - 1;
            var diffEven = (numDifferences % 2 === 0);
            if (diagonal < 0) {
                var lowerBoundEven = (diagonalsBelow % 2 === 0);
                return (diffEven === lowerBoundEven) ? 0 : 1;
            }
            else {
                var upperBoundEven = (diagonalsAbove % 2 === 0);
                return (diffEven === upperBoundEven) ? numDiagonals - 1 : numDiagonals - 2;
            }
        };
        return LcsDiff;
    }());
    exports.LcsDiff = LcsDiff;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/













define(__m[12/*vs/base/common/iterator*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.FIN = { done: true, value: undefined };
    var Iterator;
    (function (Iterator) {
        var _empty = {
            next: function () {
                return exports.FIN;
            }
        };
        function empty() {
            return _empty;
        }
        Iterator.empty = empty;
        function single(value) {
            var done = false;
            return {
                next: function () {
                    if (done) {
                        return exports.FIN;
                    }
                    done = true;
                    return { done: false, value: value };
                }
            };
        }
        Iterator.single = single;
        function fromArray(array, index, length) {
            if (index === void 0) { index = 0; }
            if (length === void 0) { length = array.length; }
            return {
                next: function () {
                    if (index >= length) {
                        return exports.FIN;
                    }
                    return { done: false, value: array[index++] };
                }
            };
        }
        Iterator.fromArray = fromArray;
        function fromNativeIterator(it) {
            return {
                next: function () {
                    var result = it.next();
                    if (result.done) {
                        return exports.FIN;
                    }
                    return { done: false, value: result.value };
                }
            };
        }
        Iterator.fromNativeIterator = fromNativeIterator;
        function from(elements) {
            if (!elements) {
                return Iterator.empty();
            }
            else if (Array.isArray(elements)) {
                return Iterator.fromArray(elements);
            }
            else {
                return elements;
            }
        }
        Iterator.from = from;
        function map(iterator, fn) {
            return {
                next: function () {
                    var element = iterator.next();
                    if (element.done) {
                        return exports.FIN;
                    }
                    else {
                        return { done: false, value: fn(element.value) };
                    }
                }
            };
        }
        Iterator.map = map;
        function filter(iterator, fn) {
            return {
                next: function () {
                    while (true) {
                        var element = iterator.next();
                        if (element.done) {
                            return exports.FIN;
                        }
                        if (fn(element.value)) {
                            return { done: false, value: element.value };
                        }
                    }
                }
            };
        }
        Iterator.filter = filter;
        function forEach(iterator, fn) {
            for (var next = iterator.next(); !next.done; next = iterator.next()) {
                fn(next.value);
            }
        }
        Iterator.forEach = forEach;
        function collect(iterator, atMost) {
            if (atMost === void 0) { atMost = Number.POSITIVE_INFINITY; }
            var result = [];
            if (atMost === 0) {
                return result;
            }
            var i = 0;
            for (var next = iterator.next(); !next.done; next = iterator.next()) {
                result.push(next.value);
                if (++i >= atMost) {
                    break;
                }
            }
            return result;
        }
        Iterator.collect = collect;
        function concat() {
            var iterators = [];
            for (var _i = 0; _i < arguments.length; _i++) {
                iterators[_i] = arguments[_i];
            }
            var i = 0;
            return {
                next: function () {
                    if (i >= iterators.length) {
                        return exports.FIN;
                    }
                    var iterator = iterators[i];
                    var result = iterator.next();
                    if (result.done) {
                        i++;
                        return this.next();
                    }
                    return result;
                }
            };
        }
        Iterator.concat = concat;
        function chain(iterator) {
            return new ChainableIterator(iterator);
        }
        Iterator.chain = chain;
    })(Iterator = exports.Iterator || (exports.Iterator = {}));
    var ChainableIterator = /** @class */ (function () {
        function ChainableIterator(it) {
            this.it = it;
        }
        ChainableIterator.prototype.next = function () { return this.it.next(); };
        ChainableIterator.prototype.map = function (fn) { return new ChainableIterator(Iterator.map(this.it, fn)); };
        ChainableIterator.prototype.filter = function (fn) { return new ChainableIterator(Iterator.filter(this.it, fn)); };
        return ChainableIterator;
    }());
    exports.ChainableIterator = ChainableIterator;
    function getSequenceIterator(arg) {
        if (Array.isArray(arg)) {
            return Iterator.fromArray(arg);
        }
        else if (!arg) {
            return Iterator.empty();
        }
        else {
            return arg;
        }
    }
    exports.getSequenceIterator = getSequenceIterator;
    var ArrayIterator = /** @class */ (function () {
        function ArrayIterator(items, start, end, index) {
            if (start === void 0) { start = 0; }
            if (end === void 0) { end = items.length; }
            if (index === void 0) { index = start - 1; }
            this.items = items;
            this.start = start;
            this.end = end;
            this.index = index;
        }
        ArrayIterator.prototype.first = function () {
            this.index = this.start;
            return this.current();
        };
        ArrayIterator.prototype.next = function () {
            this.index = Math.min(this.index + 1, this.end);
            return this.current();
        };
        ArrayIterator.prototype.current = function () {
            if (this.index === this.start - 1 || this.index === this.end) {
                return null;
            }
            return this.items[this.index];
        };
        return ArrayIterator;
    }());
    exports.ArrayIterator = ArrayIterator;
    var ArrayNavigator = /** @class */ (function (_super) {
        __extends(ArrayNavigator, _super);
        function ArrayNavigator(items, start, end, index) {
            if (start === void 0) { start = 0; }
            if (end === void 0) { end = items.length; }
            if (index === void 0) { index = start - 1; }
            return _super.call(this, items, start, end, index) || this;
        }
        ArrayNavigator.prototype.current = function () {
            return _super.prototype.current.call(this);
        };
        ArrayNavigator.prototype.previous = function () {
            this.index = Math.max(this.index - 1, this.start - 1);
            return this.current();
        };
        ArrayNavigator.prototype.first = function () {
            this.index = this.start;
            return this.current();
        };
        ArrayNavigator.prototype.last = function () {
            this.index = this.end - 1;
            return this.current();
        };
        ArrayNavigator.prototype.parent = function () {
            return null;
        };
        return ArrayNavigator;
    }(ArrayIterator));
    exports.ArrayNavigator = ArrayNavigator;
    var MappedIterator = /** @class */ (function () {
        function MappedIterator(iterator, fn) {
            this.iterator = iterator;
            this.fn = fn;
            // noop
        }
        MappedIterator.prototype.next = function () { return this.fn(this.iterator.next()); };
        return MappedIterator;
    }());
    exports.MappedIterator = MappedIterator;
    var MappedNavigator = /** @class */ (function (_super) {
        __extends(MappedNavigator, _super);
        function MappedNavigator(navigator, fn) {
            var _this = _super.call(this, navigator, fn) || this;
            _this.navigator = navigator;
            return _this;
        }
        MappedNavigator.prototype.current = function () { return this.fn(this.navigator.current()); };
        MappedNavigator.prototype.previous = function () { return this.fn(this.navigator.previous()); };
        MappedNavigator.prototype.parent = function () { return this.fn(this.navigator.parent()); };
        MappedNavigator.prototype.first = function () { return this.fn(this.navigator.first()); };
        MappedNavigator.prototype.last = function () { return this.fn(this.navigator.last()); };
        MappedNavigator.prototype.next = function () { return this.fn(this.navigator.next()); };
        return MappedNavigator;
    }(MappedIterator));
    exports.MappedNavigator = MappedNavigator;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[16/*vs/base/common/keyCodes*/], __M([1/*require*/,0/*exports*/,3/*vs/base/common/errors*/]), function (require, exports, errors_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var KeyCodeStrMap = /** @class */ (function () {
        function KeyCodeStrMap() {
            this._keyCodeToStr = [];
            this._strToKeyCode = Object.create(null);
        }
        KeyCodeStrMap.prototype.define = function (keyCode, str) {
            this._keyCodeToStr[keyCode] = str;
            this._strToKeyCode[str.toLowerCase()] = keyCode;
        };
        KeyCodeStrMap.prototype.keyCodeToStr = function (keyCode) {
            return this._keyCodeToStr[keyCode];
        };
        KeyCodeStrMap.prototype.strToKeyCode = function (str) {
            return this._strToKeyCode[str.toLowerCase()] || 0 /* Unknown */;
        };
        return KeyCodeStrMap;
    }());
    var uiMap = new KeyCodeStrMap();
    var userSettingsUSMap = new KeyCodeStrMap();
    var userSettingsGeneralMap = new KeyCodeStrMap();
    (function () {
        function define(keyCode, uiLabel, usUserSettingsLabel, generalUserSettingsLabel) {
            if (usUserSettingsLabel === void 0) { usUserSettingsLabel = uiLabel; }
            if (generalUserSettingsLabel === void 0) { generalUserSettingsLabel = usUserSettingsLabel; }
            uiMap.define(keyCode, uiLabel);
            userSettingsUSMap.define(keyCode, usUserSettingsLabel);
            userSettingsGeneralMap.define(keyCode, generalUserSettingsLabel);
        }
        define(0 /* Unknown */, 'unknown');
        define(1 /* Backspace */, 'Backspace');
        define(2 /* Tab */, 'Tab');
        define(3 /* Enter */, 'Enter');
        define(4 /* Shift */, 'Shift');
        define(5 /* Ctrl */, 'Ctrl');
        define(6 /* Alt */, 'Alt');
        define(7 /* PauseBreak */, 'PauseBreak');
        define(8 /* CapsLock */, 'CapsLock');
        define(9 /* Escape */, 'Escape');
        define(10 /* Space */, 'Space');
        define(11 /* PageUp */, 'PageUp');
        define(12 /* PageDown */, 'PageDown');
        define(13 /* End */, 'End');
        define(14 /* Home */, 'Home');
        define(15 /* LeftArrow */, 'LeftArrow', 'Left');
        define(16 /* UpArrow */, 'UpArrow', 'Up');
        define(17 /* RightArrow */, 'RightArrow', 'Right');
        define(18 /* DownArrow */, 'DownArrow', 'Down');
        define(19 /* Insert */, 'Insert');
        define(20 /* Delete */, 'Delete');
        define(21 /* KEY_0 */, '0');
        define(22 /* KEY_1 */, '1');
        define(23 /* KEY_2 */, '2');
        define(24 /* KEY_3 */, '3');
        define(25 /* KEY_4 */, '4');
        define(26 /* KEY_5 */, '5');
        define(27 /* KEY_6 */, '6');
        define(28 /* KEY_7 */, '7');
        define(29 /* KEY_8 */, '8');
        define(30 /* KEY_9 */, '9');
        define(31 /* KEY_A */, 'A');
        define(32 /* KEY_B */, 'B');
        define(33 /* KEY_C */, 'C');
        define(34 /* KEY_D */, 'D');
        define(35 /* KEY_E */, 'E');
        define(36 /* KEY_F */, 'F');
        define(37 /* KEY_G */, 'G');
        define(38 /* KEY_H */, 'H');
        define(39 /* KEY_I */, 'I');
        define(40 /* KEY_J */, 'J');
        define(41 /* KEY_K */, 'K');
        define(42 /* KEY_L */, 'L');
        define(43 /* KEY_M */, 'M');
        define(44 /* KEY_N */, 'N');
        define(45 /* KEY_O */, 'O');
        define(46 /* KEY_P */, 'P');
        define(47 /* KEY_Q */, 'Q');
        define(48 /* KEY_R */, 'R');
        define(49 /* KEY_S */, 'S');
        define(50 /* KEY_T */, 'T');
        define(51 /* KEY_U */, 'U');
        define(52 /* KEY_V */, 'V');
        define(53 /* KEY_W */, 'W');
        define(54 /* KEY_X */, 'X');
        define(55 /* KEY_Y */, 'Y');
        define(56 /* KEY_Z */, 'Z');
        define(57 /* Meta */, 'Meta');
        define(58 /* ContextMenu */, 'ContextMenu');
        define(59 /* F1 */, 'F1');
        define(60 /* F2 */, 'F2');
        define(61 /* F3 */, 'F3');
        define(62 /* F4 */, 'F4');
        define(63 /* F5 */, 'F5');
        define(64 /* F6 */, 'F6');
        define(65 /* F7 */, 'F7');
        define(66 /* F8 */, 'F8');
        define(67 /* F9 */, 'F9');
        define(68 /* F10 */, 'F10');
        define(69 /* F11 */, 'F11');
        define(70 /* F12 */, 'F12');
        define(71 /* F13 */, 'F13');
        define(72 /* F14 */, 'F14');
        define(73 /* F15 */, 'F15');
        define(74 /* F16 */, 'F16');
        define(75 /* F17 */, 'F17');
        define(76 /* F18 */, 'F18');
        define(77 /* F19 */, 'F19');
        define(78 /* NumLock */, 'NumLock');
        define(79 /* ScrollLock */, 'ScrollLock');
        define(80 /* US_SEMICOLON */, ';', ';', 'OEM_1');
        define(81 /* US_EQUAL */, '=', '=', 'OEM_PLUS');
        define(82 /* US_COMMA */, ',', ',', 'OEM_COMMA');
        define(83 /* US_MINUS */, '-', '-', 'OEM_MINUS');
        define(84 /* US_DOT */, '.', '.', 'OEM_PERIOD');
        define(85 /* US_SLASH */, '/', '/', 'OEM_2');
        define(86 /* US_BACKTICK */, '`', '`', 'OEM_3');
        define(110 /* ABNT_C1 */, 'ABNT_C1');
        define(111 /* ABNT_C2 */, 'ABNT_C2');
        define(87 /* US_OPEN_SQUARE_BRACKET */, '[', '[', 'OEM_4');
        define(88 /* US_BACKSLASH */, '\\', '\\', 'OEM_5');
        define(89 /* US_CLOSE_SQUARE_BRACKET */, ']', ']', 'OEM_6');
        define(90 /* US_QUOTE */, '\'', '\'', 'OEM_7');
        define(91 /* OEM_8 */, 'OEM_8');
        define(92 /* OEM_102 */, 'OEM_102');
        define(93 /* NUMPAD_0 */, 'NumPad0');
        define(94 /* NUMPAD_1 */, 'NumPad1');
        define(95 /* NUMPAD_2 */, 'NumPad2');
        define(96 /* NUMPAD_3 */, 'NumPad3');
        define(97 /* NUMPAD_4 */, 'NumPad4');
        define(98 /* NUMPAD_5 */, 'NumPad5');
        define(99 /* NUMPAD_6 */, 'NumPad6');
        define(100 /* NUMPAD_7 */, 'NumPad7');
        define(101 /* NUMPAD_8 */, 'NumPad8');
        define(102 /* NUMPAD_9 */, 'NumPad9');
        define(103 /* NUMPAD_MULTIPLY */, 'NumPad_Multiply');
        define(104 /* NUMPAD_ADD */, 'NumPad_Add');
        define(105 /* NUMPAD_SEPARATOR */, 'NumPad_Separator');
        define(106 /* NUMPAD_SUBTRACT */, 'NumPad_Subtract');
        define(107 /* NUMPAD_DECIMAL */, 'NumPad_Decimal');
        define(108 /* NUMPAD_DIVIDE */, 'NumPad_Divide');
    })();
    var KeyCodeUtils;
    (function (KeyCodeUtils) {
        function toString(keyCode) {
            return uiMap.keyCodeToStr(keyCode);
        }
        KeyCodeUtils.toString = toString;
        function fromString(key) {
            return uiMap.strToKeyCode(key);
        }
        KeyCodeUtils.fromString = fromString;
        function toUserSettingsUS(keyCode) {
            return userSettingsUSMap.keyCodeToStr(keyCode);
        }
        KeyCodeUtils.toUserSettingsUS = toUserSettingsUS;
        function toUserSettingsGeneral(keyCode) {
            return userSettingsGeneralMap.keyCodeToStr(keyCode);
        }
        KeyCodeUtils.toUserSettingsGeneral = toUserSettingsGeneral;
        function fromUserSettings(key) {
            return userSettingsUSMap.strToKeyCode(key) || userSettingsGeneralMap.strToKeyCode(key);
        }
        KeyCodeUtils.fromUserSettings = fromUserSettings;
    })(KeyCodeUtils = exports.KeyCodeUtils || (exports.KeyCodeUtils = {}));
    function KeyChord(firstPart, secondPart) {
        var chordPart = ((secondPart & 0x0000FFFF) << 16) >>> 0;
        return (firstPart | chordPart) >>> 0;
    }
    exports.KeyChord = KeyChord;
    function createKeybinding(keybinding, OS) {
        if (keybinding === 0) {
            return null;
        }
        var firstPart = (keybinding & 0x0000FFFF) >>> 0;
        var chordPart = (keybinding & 0xFFFF0000) >>> 16;
        if (chordPart !== 0) {
            return new ChordKeybinding([
                createSimpleKeybinding(firstPart, OS),
                createSimpleKeybinding(chordPart, OS)
            ]);
        }
        return new ChordKeybinding([createSimpleKeybinding(firstPart, OS)]);
    }
    exports.createKeybinding = createKeybinding;
    function createSimpleKeybinding(keybinding, OS) {
        var ctrlCmd = (keybinding & 2048 /* CtrlCmd */ ? true : false);
        var winCtrl = (keybinding & 256 /* WinCtrl */ ? true : false);
        var ctrlKey = (OS === 2 /* Macintosh */ ? winCtrl : ctrlCmd);
        var shiftKey = (keybinding & 1024 /* Shift */ ? true : false);
        var altKey = (keybinding & 512 /* Alt */ ? true : false);
        var metaKey = (OS === 2 /* Macintosh */ ? ctrlCmd : winCtrl);
        var keyCode = (keybinding & 255 /* KeyCode */);
        return new SimpleKeybinding(ctrlKey, shiftKey, altKey, metaKey, keyCode);
    }
    exports.createSimpleKeybinding = createSimpleKeybinding;
    var SimpleKeybinding = /** @class */ (function () {
        function SimpleKeybinding(ctrlKey, shiftKey, altKey, metaKey, keyCode) {
            this.ctrlKey = ctrlKey;
            this.shiftKey = shiftKey;
            this.altKey = altKey;
            this.metaKey = metaKey;
            this.keyCode = keyCode;
        }
        SimpleKeybinding.prototype.equals = function (other) {
            return (this.ctrlKey === other.ctrlKey
                && this.shiftKey === other.shiftKey
                && this.altKey === other.altKey
                && this.metaKey === other.metaKey
                && this.keyCode === other.keyCode);
        };
        SimpleKeybinding.prototype.getHashCode = function () {
            var ctrl = this.ctrlKey ? '1' : '0';
            var shift = this.shiftKey ? '1' : '0';
            var alt = this.altKey ? '1' : '0';
            var meta = this.metaKey ? '1' : '0';
            return "" + ctrl + shift + alt + meta + this.keyCode;
        };
        SimpleKeybinding.prototype.isModifierKey = function () {
            return (this.keyCode === 0 /* Unknown */
                || this.keyCode === 5 /* Ctrl */
                || this.keyCode === 57 /* Meta */
                || this.keyCode === 6 /* Alt */
                || this.keyCode === 4 /* Shift */);
        };
        SimpleKeybinding.prototype.toChord = function () {
            return new ChordKeybinding([this]);
        };
        /**
         * Does this keybinding refer to the key code of a modifier and it also has the modifier flag?
         */
        SimpleKeybinding.prototype.isDuplicateModifierCase = function () {
            return ((this.ctrlKey && this.keyCode === 5 /* Ctrl */)
                || (this.shiftKey && this.keyCode === 4 /* Shift */)
                || (this.altKey && this.keyCode === 6 /* Alt */)
                || (this.metaKey && this.keyCode === 57 /* Meta */));
        };
        return SimpleKeybinding;
    }());
    exports.SimpleKeybinding = SimpleKeybinding;
    var ChordKeybinding = /** @class */ (function () {
        function ChordKeybinding(parts) {
            if (parts.length === 0) {
                throw errors_1.illegalArgument("parts");
            }
            this.parts = parts;
        }
        ChordKeybinding.prototype.getHashCode = function () {
            var result = '';
            for (var i = 0, len = this.parts.length; i < len; i++) {
                if (i !== 0) {
                    result += ';';
                }
                result += this.parts[i].getHashCode();
            }
            return result;
        };
        ChordKeybinding.prototype.equals = function (other) {
            if (other === null) {
                return false;
            }
            if (this.parts.length !== other.parts.length) {
                return false;
            }
            for (var i = 0; i < this.parts.length; i++) {
                if (!this.parts[i].equals(other.parts[i])) {
                    return false;
                }
            }
            return true;
        };
        return ChordKeybinding;
    }());
    exports.ChordKeybinding = ChordKeybinding;
    var ResolvedKeybindingPart = /** @class */ (function () {
        function ResolvedKeybindingPart(ctrlKey, shiftKey, altKey, metaKey, kbLabel, kbAriaLabel) {
            this.ctrlKey = ctrlKey;
            this.shiftKey = shiftKey;
            this.altKey = altKey;
            this.metaKey = metaKey;
            this.keyLabel = kbLabel;
            this.keyAriaLabel = kbAriaLabel;
        }
        return ResolvedKeybindingPart;
    }());
    exports.ResolvedKeybindingPart = ResolvedKeybindingPart;
    /**
     * A resolved keybinding. Can be a simple keybinding or a chord keybinding.
     */
    var ResolvedKeybinding = /** @class */ (function () {
        function ResolvedKeybinding() {
        }
        return ResolvedKeybinding;
    }());
    exports.ResolvedKeybinding = ResolvedKeybinding;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[6/*vs/base/common/lifecycle*/], __M([1/*require*/,0/*exports*/,8/*vs/base/common/functional*/]), function (require, exports, functional_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * Enables logging of potentially leaked disposables.
     *
     * A disposable is considered leaked if it is not disposed or not registered as the child of
     * another disposable. This tracking is very simple an only works for classes that either
     * extend Disposable or use a DisposableStore. This means there are a lot of false positives.
     */
    var TRACK_DISPOSABLES = false;
    var __is_disposable_tracked__ = '__is_disposable_tracked__';
    function markTracked(x) {
        if (!TRACK_DISPOSABLES) {
            return;
        }
        if (x && x !== Disposable.None) {
            try {
                x[__is_disposable_tracked__] = true;
            }
            catch (_a) {
                // noop
            }
        }
    }
    function trackDisposable(x) {
        if (!TRACK_DISPOSABLES) {
            return x;
        }
        var stack = new Error('Potentially leaked disposable').stack;
        setTimeout(function () {
            if (!x[__is_disposable_tracked__]) {
                console.log(stack);
            }
        }, 3000);
        return x;
    }
    function isDisposable(thing) {
        return typeof thing.dispose === 'function'
            && thing.dispose.length === 0;
    }
    exports.isDisposable = isDisposable;
    function dispose(disposables) {
        if (Array.isArray(disposables)) {
            disposables.forEach(function (d) {
                if (d) {
                    markTracked(d);
                    d.dispose();
                }
            });
            return [];
        }
        else if (disposables) {
            markTracked(disposables);
            disposables.dispose();
            return disposables;
        }
        else {
            return undefined;
        }
    }
    exports.dispose = dispose;
    function combinedDisposable() {
        var disposables = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            disposables[_i] = arguments[_i];
        }
        disposables.forEach(markTracked);
        return trackDisposable({ dispose: function () { return dispose(disposables); } });
    }
    exports.combinedDisposable = combinedDisposable;
    function toDisposable(fn) {
        var self = trackDisposable({
            dispose: function () {
                markTracked(self);
                fn();
            }
        });
        return self;
    }
    exports.toDisposable = toDisposable;
    var DisposableStore = /** @class */ (function () {
        function DisposableStore() {
            this._toDispose = new Set();
            this._isDisposed = false;
        }
        /**
         * Dispose of all registered disposables and mark this object as disposed.
         *
         * Any future disposables added to this object will be disposed of on `add`.
         */
        DisposableStore.prototype.dispose = function () {
            if (this._isDisposed) {
                return;
            }
            markTracked(this);
            this._isDisposed = true;
            this.clear();
        };
        /**
         * Dispose of all registered disposables but do not mark this object as disposed.
         */
        DisposableStore.prototype.clear = function () {
            this._toDispose.forEach(function (item) { return item.dispose(); });
            this._toDispose.clear();
        };
        DisposableStore.prototype.add = function (t) {
            if (!t) {
                return t;
            }
            if (t === this) {
                throw new Error('Cannot register a disposable on itself!');
            }
            markTracked(t);
            if (this._isDisposed) {
                console.warn(new Error('Trying to add a disposable to a DisposableStore that has already been disposed of. The added object will be leaked!').stack);
            }
            else {
                this._toDispose.add(t);
            }
            return t;
        };
        return DisposableStore;
    }());
    exports.DisposableStore = DisposableStore;
    var Disposable = /** @class */ (function () {
        function Disposable() {
            this._store = new DisposableStore();
            trackDisposable(this);
        }
        Disposable.prototype.dispose = function () {
            markTracked(this);
            this._store.dispose();
        };
        Disposable.prototype._register = function (t) {
            if (t === this) {
                throw new Error('Cannot register a disposable on itself!');
            }
            return this._store.add(t);
        };
        Disposable.None = Object.freeze({ dispose: function () { } });
        return Disposable;
    }());
    exports.Disposable = Disposable;
    /**
     * Manages the lifecycle of a disposable value that may be changed.
     *
     * This ensures that when the disposable value is changed, the previously held disposable is disposed of. You can
     * also register a `MutableDisposable` on a `Disposable` to ensure it is automatically cleaned up.
     */
    var MutableDisposable = /** @class */ (function () {
        function MutableDisposable() {
            this._isDisposed = false;
            trackDisposable(this);
        }
        Object.defineProperty(MutableDisposable.prototype, "value", {
            get: function () {
                return this._isDisposed ? undefined : this._value;
            },
            set: function (value) {
                if (this._isDisposed || value === this._value) {
                    return;
                }
                if (this._value) {
                    this._value.dispose();
                }
                if (value) {
                    markTracked(value);
                }
                this._value = value;
            },
            enumerable: true,
            configurable: true
        });
        MutableDisposable.prototype.clear = function () {
            this.value = undefined;
        };
        MutableDisposable.prototype.dispose = function () {
            this._isDisposed = true;
            markTracked(this);
            if (this._value) {
                this._value.dispose();
            }
            this._value = undefined;
        };
        return MutableDisposable;
    }());
    exports.MutableDisposable = MutableDisposable;
    var ReferenceCollection = /** @class */ (function () {
        function ReferenceCollection() {
            this.references = new Map();
        }
        ReferenceCollection.prototype.acquire = function (key) {
            var _this = this;
            var reference = this.references.get(key);
            if (!reference) {
                reference = { counter: 0, object: this.createReferencedObject(key) };
                this.references.set(key, reference);
            }
            var object = reference.object;
            var dispose = functional_1.once(function () {
                if (--reference.counter === 0) {
                    _this.destroyReferencedObject(key, reference.object);
                    _this.references.delete(key);
                }
            });
            reference.counter++;
            return { object: object, dispose: dispose };
        };
        return ReferenceCollection;
    }());
    exports.ReferenceCollection = ReferenceCollection;
    var ImmortalReference = /** @class */ (function () {
        function ImmortalReference(object) {
            this.object = object;
        }
        ImmortalReference.prototype.dispose = function () { };
        return ImmortalReference;
    }());
    exports.ImmortalReference = ImmortalReference;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[18/*vs/base/common/linkedList*/], __M([1/*require*/,0/*exports*/,12/*vs/base/common/iterator*/]), function (require, exports, iterator_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var Node = /** @class */ (function () {
        function Node(element) {
            this.element = element;
            this.next = Node.Undefined;
            this.prev = Node.Undefined;
        }
        Node.Undefined = new Node(undefined);
        return Node;
    }());
    var LinkedList = /** @class */ (function () {
        function LinkedList() {
            this._first = Node.Undefined;
            this._last = Node.Undefined;
            this._size = 0;
        }
        Object.defineProperty(LinkedList.prototype, "size", {
            get: function () {
                return this._size;
            },
            enumerable: true,
            configurable: true
        });
        LinkedList.prototype.isEmpty = function () {
            return this._first === Node.Undefined;
        };
        LinkedList.prototype.clear = function () {
            this._first = Node.Undefined;
            this._last = Node.Undefined;
            this._size = 0;
        };
        LinkedList.prototype.unshift = function (element) {
            return this._insert(element, false);
        };
        LinkedList.prototype.push = function (element) {
            return this._insert(element, true);
        };
        LinkedList.prototype._insert = function (element, atTheEnd) {
            var _this = this;
            var newNode = new Node(element);
            if (this._first === Node.Undefined) {
                this._first = newNode;
                this._last = newNode;
            }
            else if (atTheEnd) {
                // push
                var oldLast = this._last;
                this._last = newNode;
                newNode.prev = oldLast;
                oldLast.next = newNode;
            }
            else {
                // unshift
                var oldFirst = this._first;
                this._first = newNode;
                newNode.next = oldFirst;
                oldFirst.prev = newNode;
            }
            this._size += 1;
            var didRemove = false;
            return function () {
                if (!didRemove) {
                    didRemove = true;
                    _this._remove(newNode);
                }
            };
        };
        LinkedList.prototype.shift = function () {
            if (this._first === Node.Undefined) {
                return undefined;
            }
            else {
                var res = this._first.element;
                this._remove(this._first);
                return res;
            }
        };
        LinkedList.prototype.pop = function () {
            if (this._last === Node.Undefined) {
                return undefined;
            }
            else {
                var res = this._last.element;
                this._remove(this._last);
                return res;
            }
        };
        LinkedList.prototype._remove = function (node) {
            if (node.prev !== Node.Undefined && node.next !== Node.Undefined) {
                // middle
                var anchor = node.prev;
                anchor.next = node.next;
                node.next.prev = anchor;
            }
            else if (node.prev === Node.Undefined && node.next === Node.Undefined) {
                // only node
                this._first = Node.Undefined;
                this._last = Node.Undefined;
            }
            else if (node.next === Node.Undefined) {
                // last
                this._last = this._last.prev;
                this._last.next = Node.Undefined;
            }
            else if (node.prev === Node.Undefined) {
                // first
                this._first = this._first.next;
                this._first.prev = Node.Undefined;
            }
            // done
            this._size -= 1;
        };
        LinkedList.prototype.iterator = function () {
            var element;
            var node = this._first;
            return {
                next: function () {
                    if (node === Node.Undefined) {
                        return iterator_1.FIN;
                    }
                    if (!element) {
                        element = { done: false, value: node.element };
                    }
                    else {
                        element.value = node.element;
                    }
                    node = node.next;
                    return element;
                }
            };
        };
        LinkedList.prototype.toArray = function () {
            var result = [];
            for (var node = this._first; node !== Node.Undefined; node = node.next) {
                result.push(node.element);
            }
            return result;
        };
        return LinkedList;
    }());
    exports.LinkedList = LinkedList;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/













var __assign = (this && this.__assign) || function () {
    __assign = Object.assign || function(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
                t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};




































define(__m[13/*vs/base/common/event*/], __M([1/*require*/,0/*exports*/,3/*vs/base/common/errors*/,8/*vs/base/common/functional*/,6/*vs/base/common/lifecycle*/,18/*vs/base/common/linkedList*/]), function (require, exports, errors_1, functional_1, lifecycle_1, linkedList_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var Event;
    (function (Event) {
        Event.None = function () { return lifecycle_1.Disposable.None; };
        /**
         * Given an event, returns another event which only fires once.
         */
        function once(event) {
            return function (listener, thisArgs, disposables) {
                if (thisArgs === void 0) { thisArgs = null; }
                // we need this, in case the event fires during the listener call
                var didFire = false;
                var result;
                result = event(function (e) {
                    if (didFire) {
                        return;
                    }
                    else if (result) {
                        result.dispose();
                    }
                    else {
                        didFire = true;
                    }
                    return listener.call(thisArgs, e);
                }, null, disposables);
                if (didFire) {
                    result.dispose();
                }
                return result;
            };
        }
        Event.once = once;
        /**
         * Given an event and a `map` function, returns another event which maps each element
         * through the mapping function.
         */
        function map(event, map) {
            return snapshot(function (listener, thisArgs, disposables) {
                if (thisArgs === void 0) { thisArgs = null; }
                return event(function (i) { return listener.call(thisArgs, map(i)); }, null, disposables);
            });
        }
        Event.map = map;
        /**
         * Given an event and an `each` function, returns another identical event and calls
         * the `each` function per each element.
         */
        function forEach(event, each) {
            return snapshot(function (listener, thisArgs, disposables) {
                if (thisArgs === void 0) { thisArgs = null; }
                return event(function (i) { each(i); listener.call(thisArgs, i); }, null, disposables);
            });
        }
        Event.forEach = forEach;
        function filter(event, filter) {
            return snapshot(function (listener, thisArgs, disposables) {
                if (thisArgs === void 0) { thisArgs = null; }
                return event(function (e) { return filter(e) && listener.call(thisArgs, e); }, null, disposables);
            });
        }
        Event.filter = filter;
        /**
         * Given an event, returns the same event but typed as `Event<void>`.
         */
        function signal(event) {
            return event;
        }
        Event.signal = signal;
        /**
         * Given a collection of events, returns a single event which emits
         * whenever any of the provided events emit.
         */
        function any() {
            var events = [];
            for (var _i = 0; _i < arguments.length; _i++) {
                events[_i] = arguments[_i];
            }
            return function (listener, thisArgs, disposables) {
                if (thisArgs === void 0) { thisArgs = null; }
                return lifecycle_1.combinedDisposable.apply(void 0, events.map(function (event) { return event(function (e) { return listener.call(thisArgs, e); }, null, disposables); }));
            };
        }
        Event.any = any;
        /**
         * Given an event and a `merge` function, returns another event which maps each element
         * and the cumulative result through the `merge` function. Similar to `map`, but with memory.
         */
        function reduce(event, merge, initial) {
            var output = initial;
            return map(event, function (e) {
                output = merge(output, e);
                return output;
            });
        }
        Event.reduce = reduce;
        /**
         * Given a chain of event processing functions (filter, map, etc), each
         * function will be invoked per event & per listener. Snapshotting an event
         * chain allows each function to be invoked just once per event.
         */
        function snapshot(event) {
            var listener;
            var emitter = new Emitter({
                onFirstListenerAdd: function () {
                    listener = event(emitter.fire, emitter);
                },
                onLastListenerRemove: function () {
                    listener.dispose();
                }
            });
            return emitter.event;
        }
        Event.snapshot = snapshot;
        function debounce(event, merge, delay, leading, leakWarningThreshold) {
            if (delay === void 0) { delay = 100; }
            if (leading === void 0) { leading = false; }
            var subscription;
            var output = undefined;
            var handle = undefined;
            var numDebouncedCalls = 0;
            var emitter = new Emitter({
                leakWarningThreshold: leakWarningThreshold,
                onFirstListenerAdd: function () {
                    subscription = event(function (cur) {
                        numDebouncedCalls++;
                        output = merge(output, cur);
                        if (leading && !handle) {
                            emitter.fire(output);
                            output = undefined;
                        }
                        clearTimeout(handle);
                        handle = setTimeout(function () {
                            var _output = output;
                            output = undefined;
                            handle = undefined;
                            if (!leading || numDebouncedCalls > 1) {
                                emitter.fire(_output);
                            }
                            numDebouncedCalls = 0;
                        }, delay);
                    });
                },
                onLastListenerRemove: function () {
                    subscription.dispose();
                }
            });
            return emitter.event;
        }
        Event.debounce = debounce;
        /**
         * Given an event, it returns another event which fires only once and as soon as
         * the input event emits. The event data is the number of millis it took for the
         * event to fire.
         */
        function stopwatch(event) {
            var start = new Date().getTime();
            return map(once(event), function (_) { return new Date().getTime() - start; });
        }
        Event.stopwatch = stopwatch;
        /**
         * Given an event, it returns another event which fires only when the event
         * element changes.
         */
        function latch(event) {
            var firstCall = true;
            var cache;
            return filter(event, function (value) {
                var shouldEmit = firstCall || value !== cache;
                firstCall = false;
                cache = value;
                return shouldEmit;
            });
        }
        Event.latch = latch;
        /**
         * Buffers the provided event until a first listener comes
         * along, at which point fire all the events at once and
         * pipe the event from then on.
         *
         * ```typescript
         * const emitter = new Emitter<number>();
         * const event = emitter.event;
         * const bufferedEvent = buffer(event);
         *
         * emitter.fire(1);
         * emitter.fire(2);
         * emitter.fire(3);
         * // nothing...
         *
         * const listener = bufferedEvent(num => console.log(num));
         * // 1, 2, 3
         *
         * emitter.fire(4);
         * // 4
         * ```
         */
        function buffer(event, nextTick, _buffer) {
            if (nextTick === void 0) { nextTick = false; }
            if (_buffer === void 0) { _buffer = []; }
            var buffer = _buffer.slice();
            var listener = event(function (e) {
                if (buffer) {
                    buffer.push(e);
                }
                else {
                    emitter.fire(e);
                }
            });
            var flush = function () {
                if (buffer) {
                    buffer.forEach(function (e) { return emitter.fire(e); });
                }
                buffer = null;
            };
            var emitter = new Emitter({
                onFirstListenerAdd: function () {
                    if (!listener) {
                        listener = event(function (e) { return emitter.fire(e); });
                    }
                },
                onFirstListenerDidAdd: function () {
                    if (buffer) {
                        if (nextTick) {
                            setTimeout(flush);
                        }
                        else {
                            flush();
                        }
                    }
                },
                onLastListenerRemove: function () {
                    if (listener) {
                        listener.dispose();
                    }
                    listener = null;
                }
            });
            return emitter.event;
        }
        Event.buffer = buffer;
        var ChainableEvent = /** @class */ (function () {
            function ChainableEvent(event) {
                this.event = event;
            }
            ChainableEvent.prototype.map = function (fn) {
                return new ChainableEvent(map(this.event, fn));
            };
            ChainableEvent.prototype.forEach = function (fn) {
                return new ChainableEvent(forEach(this.event, fn));
            };
            ChainableEvent.prototype.filter = function (fn) {
                return new ChainableEvent(filter(this.event, fn));
            };
            ChainableEvent.prototype.reduce = function (merge, initial) {
                return new ChainableEvent(reduce(this.event, merge, initial));
            };
            ChainableEvent.prototype.latch = function () {
                return new ChainableEvent(latch(this.event));
            };
            ChainableEvent.prototype.debounce = function (merge, delay, leading, leakWarningThreshold) {
                if (delay === void 0) { delay = 100; }
                if (leading === void 0) { leading = false; }
                return new ChainableEvent(debounce(this.event, merge, delay, leading, leakWarningThreshold));
            };
            ChainableEvent.prototype.on = function (listener, thisArgs, disposables) {
                return this.event(listener, thisArgs, disposables);
            };
            ChainableEvent.prototype.once = function (listener, thisArgs, disposables) {
                return once(this.event)(listener, thisArgs, disposables);
            };
            return ChainableEvent;
        }());
        function chain(event) {
            return new ChainableEvent(event);
        }
        Event.chain = chain;
        function fromNodeEventEmitter(emitter, eventName, map) {
            if (map === void 0) { map = function (id) { return id; }; }
            var fn = function () {
                var args = [];
                for (var _i = 0; _i < arguments.length; _i++) {
                    args[_i] = arguments[_i];
                }
                return result.fire(map.apply(void 0, args));
            };
            var onFirstListenerAdd = function () { return emitter.on(eventName, fn); };
            var onLastListenerRemove = function () { return emitter.removeListener(eventName, fn); };
            var result = new Emitter({ onFirstListenerAdd: onFirstListenerAdd, onLastListenerRemove: onLastListenerRemove });
            return result.event;
        }
        Event.fromNodeEventEmitter = fromNodeEventEmitter;
        function fromDOMEventEmitter(emitter, eventName, map) {
            if (map === void 0) { map = function (id) { return id; }; }
            var fn = function () {
                var args = [];
                for (var _i = 0; _i < arguments.length; _i++) {
                    args[_i] = arguments[_i];
                }
                return result.fire(map.apply(void 0, args));
            };
            var onFirstListenerAdd = function () { return emitter.addEventListener(eventName, fn); };
            var onLastListenerRemove = function () { return emitter.removeEventListener(eventName, fn); };
            var result = new Emitter({ onFirstListenerAdd: onFirstListenerAdd, onLastListenerRemove: onLastListenerRemove });
            return result.event;
        }
        Event.fromDOMEventEmitter = fromDOMEventEmitter;
        function fromPromise(promise) {
            var emitter = new Emitter();
            var shouldEmit = false;
            promise
                .then(undefined, function () { return null; })
                .then(function () {
                if (!shouldEmit) {
                    setTimeout(function () { return emitter.fire(undefined); }, 0);
                }
                else {
                    emitter.fire(undefined);
                }
            });
            shouldEmit = true;
            return emitter.event;
        }
        Event.fromPromise = fromPromise;
        function toPromise(event) {
            return new Promise(function (c) { return once(event)(c); });
        }
        Event.toPromise = toPromise;
    })(Event = exports.Event || (exports.Event = {}));
    var _globalLeakWarningThreshold = -1;
    function setGlobalLeakWarningThreshold(n) {
        var oldValue = _globalLeakWarningThreshold;
        _globalLeakWarningThreshold = n;
        return {
            dispose: function () {
                _globalLeakWarningThreshold = oldValue;
            }
        };
    }
    exports.setGlobalLeakWarningThreshold = setGlobalLeakWarningThreshold;
    var LeakageMonitor = /** @class */ (function () {
        function LeakageMonitor(customThreshold, name) {
            if (name === void 0) { name = Math.random().toString(18).slice(2, 5); }
            this.customThreshold = customThreshold;
            this.name = name;
            this._warnCountdown = 0;
        }
        LeakageMonitor.prototype.dispose = function () {
            if (this._stacks) {
                this._stacks.clear();
            }
        };
        LeakageMonitor.prototype.check = function (listenerCount) {
            var _this = this;
            var threshold = _globalLeakWarningThreshold;
            if (typeof this.customThreshold === 'number') {
                threshold = this.customThreshold;
            }
            if (threshold <= 0 || listenerCount < threshold) {
                return undefined;
            }
            if (!this._stacks) {
                this._stacks = new Map();
            }
            var stack = new Error().stack.split('\n').slice(3).join('\n');
            var count = (this._stacks.get(stack) || 0);
            this._stacks.set(stack, count + 1);
            this._warnCountdown -= 1;
            if (this._warnCountdown <= 0) {
                // only warn on first exceed and then every time the limit
                // is exceeded by 50% again
                this._warnCountdown = threshold * 0.5;
                // find most frequent listener and print warning
                var topStack_1;
                var topCount_1 = 0;
                this._stacks.forEach(function (count, stack) {
                    if (!topStack_1 || topCount_1 < count) {
                        topStack_1 = stack;
                        topCount_1 = count;
                    }
                });
                console.warn("[" + this.name + "] potential listener LEAK detected, having " + listenerCount + " listeners already. MOST frequent listener (" + topCount_1 + "):");
                console.warn(topStack_1);
            }
            return function () {
                var count = (_this._stacks.get(stack) || 0);
                _this._stacks.set(stack, count - 1);
            };
        };
        return LeakageMonitor;
    }());
    /**
     * The Emitter can be used to expose an Event to the public
     * to fire it from the insides.
     * Sample:
        class Document {
    
            private readonly _onDidChange = new Emitter<(value:string)=>any>();
    
            public onDidChange = this._onDidChange.event;
    
            // getter-style
            // get onDidChange(): Event<(value:string)=>any> {
            // 	return this._onDidChange.event;
            // }
    
            private _doIt() {
                //...
                this._onDidChange.fire(value);
            }
        }
     */
    var Emitter = /** @class */ (function () {
        function Emitter(options) {
            this._disposed = false;
            this._options = options;
            this._leakageMon = _globalLeakWarningThreshold > 0
                ? new LeakageMonitor(this._options && this._options.leakWarningThreshold)
                : undefined;
        }
        Object.defineProperty(Emitter.prototype, "event", {
            /**
             * For the public to allow to subscribe
             * to events from this Emitter
             */
            get: function () {
                var _this = this;
                if (!this._event) {
                    this._event = function (listener, thisArgs, disposables) {
                        if (!_this._listeners) {
                            _this._listeners = new linkedList_1.LinkedList();
                        }
                        var firstListener = _this._listeners.isEmpty();
                        if (firstListener && _this._options && _this._options.onFirstListenerAdd) {
                            _this._options.onFirstListenerAdd(_this);
                        }
                        var remove = _this._listeners.push(!thisArgs ? listener : [listener, thisArgs]);
                        if (firstListener && _this._options && _this._options.onFirstListenerDidAdd) {
                            _this._options.onFirstListenerDidAdd(_this);
                        }
                        if (_this._options && _this._options.onListenerDidAdd) {
                            _this._options.onListenerDidAdd(_this, listener, thisArgs);
                        }
                        // check and record this emitter for potential leakage
                        var removeMonitor;
                        if (_this._leakageMon) {
                            removeMonitor = _this._leakageMon.check(_this._listeners.size);
                        }
                        var result;
                        result = {
                            dispose: function () {
                                if (removeMonitor) {
                                    removeMonitor();
                                }
                                result.dispose = Emitter._noop;
                                if (!_this._disposed) {
                                    remove();
                                    if (_this._options && _this._options.onLastListenerRemove) {
                                        var hasListeners = (_this._listeners && !_this._listeners.isEmpty());
                                        if (!hasListeners) {
                                            _this._options.onLastListenerRemove(_this);
                                        }
                                    }
                                }
                            }
                        };
                        if (disposables instanceof lifecycle_1.DisposableStore) {
                            disposables.add(result);
                        }
                        else if (Array.isArray(disposables)) {
                            disposables.push(result);
                        }
                        return result;
                    };
                }
                return this._event;
            },
            enumerable: true,
            configurable: true
        });
        /**
         * To be kept private to fire an event to
         * subscribers
         */
        Emitter.prototype.fire = function (event) {
            if (this._listeners) {
                // put all [listener,event]-pairs into delivery queue
                // then emit all event. an inner/nested event might be
                // the driver of this
                if (!this._deliveryQueue) {
                    this._deliveryQueue = new linkedList_1.LinkedList();
                }
                for (var iter = this._listeners.iterator(), e = iter.next(); !e.done; e = iter.next()) {
                    this._deliveryQueue.push([e.value, event]);
                }
                while (this._deliveryQueue.size > 0) {
                    var _a = this._deliveryQueue.shift(), listener = _a[0], event_1 = _a[1];
                    try {
                        if (typeof listener === 'function') {
                            listener.call(undefined, event_1);
                        }
                        else {
                            listener[0].call(listener[1], event_1);
                        }
                    }
                    catch (e) {
                        errors_1.onUnexpectedError(e);
                    }
                }
            }
        };
        Emitter.prototype.dispose = function () {
            if (this._listeners) {
                this._listeners.clear();
            }
            if (this._deliveryQueue) {
                this._deliveryQueue.clear();
            }
            if (this._leakageMon) {
                this._leakageMon.dispose();
            }
            this._disposed = true;
        };
        Emitter._noop = function () { };
        return Emitter;
    }());
    exports.Emitter = Emitter;
    var PauseableEmitter = /** @class */ (function (_super) {
        __extends(PauseableEmitter, _super);
        function PauseableEmitter(options) {
            var _this = _super.call(this, options) || this;
            _this._isPaused = 0;
            _this._eventQueue = new linkedList_1.LinkedList();
            _this._mergeFn = options && options.merge;
            return _this;
        }
        PauseableEmitter.prototype.pause = function () {
            this._isPaused++;
        };
        PauseableEmitter.prototype.resume = function () {
            if (this._isPaused !== 0 && --this._isPaused === 0) {
                if (this._mergeFn) {
                    // use the merge function to create a single composite
                    // event. make a copy in case firing pauses this emitter
                    var events = this._eventQueue.toArray();
                    this._eventQueue.clear();
                    _super.prototype.fire.call(this, this._mergeFn(events));
                }
                else {
                    // no merging, fire each event individually and test
                    // that this emitter isn't paused halfway through
                    while (!this._isPaused && this._eventQueue.size !== 0) {
                        _super.prototype.fire.call(this, this._eventQueue.shift());
                    }
                }
            }
        };
        PauseableEmitter.prototype.fire = function (event) {
            if (this._listeners) {
                if (this._isPaused !== 0) {
                    this._eventQueue.push(event);
                }
                else {
                    _super.prototype.fire.call(this, event);
                }
            }
        };
        return PauseableEmitter;
    }(Emitter));
    exports.PauseableEmitter = PauseableEmitter;
    var AsyncEmitter = /** @class */ (function (_super) {
        __extends(AsyncEmitter, _super);
        function AsyncEmitter() {
            return _super !== null && _super.apply(this, arguments) || this;
        }
        AsyncEmitter.prototype.fireAsync = function (data, token, promiseJoin) {
            return __awaiter(this, void 0, void 0, function () {
                var iter, e, _loop_1, this_1;
                return __generator(this, function (_a) {
                    switch (_a.label) {
                        case 0:
                            if (!this._listeners) {
                                return [2 /*return*/];
                            }
                            if (!this._asyncDeliveryQueue) {
                                this._asyncDeliveryQueue = new linkedList_1.LinkedList();
                            }
                            for (iter = this._listeners.iterator(), e = iter.next(); !e.done; e = iter.next()) {
                                this._asyncDeliveryQueue.push([e.value, data]);
                            }
                            _loop_1 = function () {
                                var _a, listener, data_1, thenables, event_2;
                                return __generator(this, function (_b) {
                                    switch (_b.label) {
                                        case 0:
                                            _a = this_1._asyncDeliveryQueue.shift(), listener = _a[0], data_1 = _a[1];
                                            thenables = [];
                                            event_2 = __assign(__assign({}, data_1), { waitUntil: function (p) {
                                                    if (Object.isFrozen(thenables)) {
                                                        throw new Error('waitUntil can NOT be called asynchronous');
                                                    }
                                                    if (promiseJoin) {
                                                        p = promiseJoin(p, typeof listener === 'function' ? listener : listener[0]);
                                                    }
                                                    thenables.push(p);
                                                } });
                                            try {
                                                if (typeof listener === 'function') {
                                                    listener.call(undefined, event_2);
                                                }
                                                else {
                                                    listener[0].call(listener[1], event_2);
                                                }
                                            }
                                            catch (e) {
                                                errors_1.onUnexpectedError(e);
                                                return [2 /*return*/, "continue"];
                                            }
                                            // freeze thenables-collection to enforce sync-calls to
                                            // wait until and then wait for all thenables to resolve
                                            Object.freeze(thenables);
                                            return [4 /*yield*/, Promise.all(thenables).catch(function (e) { return errors_1.onUnexpectedError(e); })];
                                        case 1:
                                            _b.sent();
                                            return [2 /*return*/];
                                    }
                                });
                            };
                            this_1 = this;
                            _a.label = 1;
                        case 1:
                            if (!(this._asyncDeliveryQueue.size > 0 && !token.isCancellationRequested)) return [3 /*break*/, 3];
                            return [5 /*yield**/, _loop_1()];
                        case 2:
                            _a.sent();
                            return [3 /*break*/, 1];
                        case 3: return [2 /*return*/];
                    }
                });
            });
        };
        return AsyncEmitter;
    }(Emitter));
    exports.AsyncEmitter = AsyncEmitter;
    var EventMultiplexer = /** @class */ (function () {
        function EventMultiplexer() {
            var _this = this;
            this.hasListeners = false;
            this.events = [];
            this.emitter = new Emitter({
                onFirstListenerAdd: function () { return _this.onFirstListenerAdd(); },
                onLastListenerRemove: function () { return _this.onLastListenerRemove(); }
            });
        }
        Object.defineProperty(EventMultiplexer.prototype, "event", {
            get: function () {
                return this.emitter.event;
            },
            enumerable: true,
            configurable: true
        });
        EventMultiplexer.prototype.add = function (event) {
            var _this = this;
            var e = { event: event, listener: null };
            this.events.push(e);
            if (this.hasListeners) {
                this.hook(e);
            }
            var dispose = function () {
                if (_this.hasListeners) {
                    _this.unhook(e);
                }
                var idx = _this.events.indexOf(e);
                _this.events.splice(idx, 1);
            };
            return lifecycle_1.toDisposable(functional_1.once(dispose));
        };
        EventMultiplexer.prototype.onFirstListenerAdd = function () {
            var _this = this;
            this.hasListeners = true;
            this.events.forEach(function (e) { return _this.hook(e); });
        };
        EventMultiplexer.prototype.onLastListenerRemove = function () {
            var _this = this;
            this.hasListeners = false;
            this.events.forEach(function (e) { return _this.unhook(e); });
        };
        EventMultiplexer.prototype.hook = function (e) {
            var _this = this;
            e.listener = e.event(function (r) { return _this.emitter.fire(r); });
        };
        EventMultiplexer.prototype.unhook = function (e) {
            if (e.listener) {
                e.listener.dispose();
            }
            e.listener = null;
        };
        EventMultiplexer.prototype.dispose = function () {
            this.emitter.dispose();
        };
        return EventMultiplexer;
    }());
    exports.EventMultiplexer = EventMultiplexer;
    /**
     * The EventBufferer is useful in situations in which you want
     * to delay firing your events during some code.
     * You can wrap that code and be sure that the event will not
     * be fired during that wrap.
     *
     * ```
     * const emitter: Emitter;
     * const delayer = new EventDelayer();
     * const delayedEvent = delayer.wrapEvent(emitter.event);
     *
     * delayedEvent(console.log);
     *
     * delayer.bufferEvents(() => {
     *   emitter.fire(); // event will not be fired yet
     * });
     *
     * // event will only be fired at this point
     * ```
     */
    var EventBufferer = /** @class */ (function () {
        function EventBufferer() {
            this.buffers = [];
        }
        EventBufferer.prototype.wrapEvent = function (event) {
            var _this = this;
            return function (listener, thisArgs, disposables) {
                return event(function (i) {
                    var buffer = _this.buffers[_this.buffers.length - 1];
                    if (buffer) {
                        buffer.push(function () { return listener.call(thisArgs, i); });
                    }
                    else {
                        listener.call(thisArgs, i);
                    }
                }, undefined, disposables);
            };
        };
        EventBufferer.prototype.bufferEvents = function (fn) {
            var buffer = [];
            this.buffers.push(buffer);
            var r = fn();
            this.buffers.pop();
            buffer.forEach(function (flush) { return flush(); });
            return r;
        };
        return EventBufferer;
    }());
    exports.EventBufferer = EventBufferer;
    /**
     * A Relay is an event forwarder which functions as a replugabble event pipe.
     * Once created, you can connect an input event to it and it will simply forward
     * events from that input event through its own `event` property. The `input`
     * can be changed at any point in time.
     */
    var Relay = /** @class */ (function () {
        function Relay() {
            var _this = this;
            this.listening = false;
            this.inputEvent = Event.None;
            this.inputEventListener = lifecycle_1.Disposable.None;
            this.emitter = new Emitter({
                onFirstListenerDidAdd: function () {
                    _this.listening = true;
                    _this.inputEventListener = _this.inputEvent(_this.emitter.fire, _this.emitter);
                },
                onLastListenerRemove: function () {
                    _this.listening = false;
                    _this.inputEventListener.dispose();
                }
            });
            this.event = this.emitter.event;
        }
        Object.defineProperty(Relay.prototype, "input", {
            set: function (event) {
                this.inputEvent = event;
                if (this.listening) {
                    this.inputEventListener.dispose();
                    this.inputEventListener = event(this.emitter.fire, this.emitter);
                }
            },
            enumerable: true,
            configurable: true
        });
        Relay.prototype.dispose = function () {
            this.inputEventListener.dispose();
            this.emitter.dispose();
        };
        return Relay;
    }());
    exports.Relay = Relay;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[22/*vs/base/common/cancellation*/], __M([1/*require*/,0/*exports*/,13/*vs/base/common/event*/]), function (require, exports, event_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var shortcutEvent = Object.freeze(function (callback, context) {
        var handle = setTimeout(callback.bind(context), 0);
        return { dispose: function () { clearTimeout(handle); } };
    });
    var CancellationToken;
    (function (CancellationToken) {
        function isCancellationToken(thing) {
            if (thing === CancellationToken.None || thing === CancellationToken.Cancelled) {
                return true;
            }
            if (thing instanceof MutableToken) {
                return true;
            }
            if (!thing || typeof thing !== 'object') {
                return false;
            }
            return typeof thing.isCancellationRequested === 'boolean'
                && typeof thing.onCancellationRequested === 'function';
        }
        CancellationToken.isCancellationToken = isCancellationToken;
        CancellationToken.None = Object.freeze({
            isCancellationRequested: false,
            onCancellationRequested: event_1.Event.None
        });
        CancellationToken.Cancelled = Object.freeze({
            isCancellationRequested: true,
            onCancellationRequested: shortcutEvent
        });
    })(CancellationToken = exports.CancellationToken || (exports.CancellationToken = {}));
    var MutableToken = /** @class */ (function () {
        function MutableToken() {
            this._isCancelled = false;
            this._emitter = null;
        }
        MutableToken.prototype.cancel = function () {
            if (!this._isCancelled) {
                this._isCancelled = true;
                if (this._emitter) {
                    this._emitter.fire(undefined);
                    this.dispose();
                }
            }
        };
        Object.defineProperty(MutableToken.prototype, "isCancellationRequested", {
            get: function () {
                return this._isCancelled;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(MutableToken.prototype, "onCancellationRequested", {
            get: function () {
                if (this._isCancelled) {
                    return shortcutEvent;
                }
                if (!this._emitter) {
                    this._emitter = new event_1.Emitter();
                }
                return this._emitter.event;
            },
            enumerable: true,
            configurable: true
        });
        MutableToken.prototype.dispose = function () {
            if (this._emitter) {
                this._emitter.dispose();
                this._emitter = null;
            }
        };
        return MutableToken;
    }());
    var CancellationTokenSource = /** @class */ (function () {
        function CancellationTokenSource(parent) {
            this._token = undefined;
            this._parentListener = undefined;
            this._parentListener = parent && parent.onCancellationRequested(this.cancel, this);
        }
        Object.defineProperty(CancellationTokenSource.prototype, "token", {
            get: function () {
                if (!this._token) {
                    // be lazy and create the token only when
                    // actually needed
                    this._token = new MutableToken();
                }
                return this._token;
            },
            enumerable: true,
            configurable: true
        });
        CancellationTokenSource.prototype.cancel = function () {
            if (!this._token) {
                // save an object by returning the default
                // cancelled token when cancellation happens
                // before someone asks for the token
                this._token = CancellationToken.Cancelled;
            }
            else if (this._token instanceof MutableToken) {
                // actually cancel
                this._token.cancel();
            }
        };
        CancellationTokenSource.prototype.dispose = function (cancel) {
            if (cancel === void 0) { cancel = false; }
            if (cancel) {
                this.cancel();
            }
            if (this._parentListener) {
                this._parentListener.dispose();
            }
            if (!this._token) {
                // ensure to initialize with an empty token if we had none
                this._token = CancellationToken.None;
            }
            else if (this._token instanceof MutableToken) {
                // actually dispose
                this._token.dispose();
            }
        };
        return CancellationTokenSource;
    }());
    exports.CancellationTokenSource = CancellationTokenSource;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[5/*vs/base/common/platform*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var LANGUAGE_DEFAULT = 'en';
    var _isWindows = false;
    var _isMacintosh = false;
    var _isLinux = false;
    var _isNative = false;
    var _isWeb = false;
    var _isIOS = false;
    var _locale = undefined;
    var _language = LANGUAGE_DEFAULT;
    var _translationsConfigFile = undefined;
    var _userAgent = undefined;
    var isElectronRenderer = (typeof process !== 'undefined' && typeof process.versions !== 'undefined' && typeof process.versions.electron !== 'undefined' && process.type === 'renderer');
    // OS detection
    if (typeof navigator === 'object' && !isElectronRenderer) {
        _userAgent = navigator.userAgent;
        _isWindows = _userAgent.indexOf('Windows') >= 0;
        _isMacintosh = _userAgent.indexOf('Macintosh') >= 0;
        _isIOS = _userAgent.indexOf('Macintosh') >= 0 && !!navigator.maxTouchPoints && navigator.maxTouchPoints > 0;
        _isLinux = _userAgent.indexOf('Linux') >= 0;
        _isWeb = true;
        _locale = navigator.language;
        _language = _locale;
    }
    else if (typeof process === 'object') {
        _isWindows = (process.platform === 'win32');
        _isMacintosh = (process.platform === 'darwin');
        _isLinux = (process.platform === 'linux');
        _locale = LANGUAGE_DEFAULT;
        _language = LANGUAGE_DEFAULT;
        var rawNlsConfig = process.env['VSCODE_NLS_CONFIG'];
        if (rawNlsConfig) {
            try {
                var nlsConfig = JSON.parse(rawNlsConfig);
                var resolved = nlsConfig.availableLanguages['*'];
                _locale = nlsConfig.locale;
                // VSCode's default language is 'en'
                _language = resolved ? resolved : LANGUAGE_DEFAULT;
                _translationsConfigFile = nlsConfig._translationsConfigFile;
            }
            catch (e) {
            }
        }
        _isNative = true;
    }
    function PlatformToString(platform) {
        switch (platform) {
            case 0 /* Web */: return 'Web';
            case 1 /* Mac */: return 'Mac';
            case 2 /* Linux */: return 'Linux';
            case 3 /* Windows */: return 'Windows';
        }
    }
    exports.PlatformToString = PlatformToString;
    var _platform = 0 /* Web */;
    if (_isMacintosh) {
        _platform = 1 /* Mac */;
    }
    else if (_isWindows) {
        _platform = 3 /* Windows */;
    }
    else if (_isLinux) {
        _platform = 2 /* Linux */;
    }
    exports.isWindows = _isWindows;
    exports.isMacintosh = _isMacintosh;
    exports.isLinux = _isLinux;
    exports.isNative = _isNative;
    exports.isWeb = _isWeb;
    exports.isIOS = _isIOS;
    exports.platform = _platform;
    exports.userAgent = _userAgent;
    function isRootUser() {
        return _isNative && !_isWindows && (process.getuid() === 0);
    }
    exports.isRootUser = isRootUser;
    /**
     * The language used for the user interface. The format of
     * the string is all lower case (e.g. zh-tw for Traditional
     * Chinese)
     */
    exports.language = _language;
    var Language;
    (function (Language) {
        function value() {
            return exports.language;
        }
        Language.value = value;
        function isDefaultVariant() {
            if (exports.language.length === 2) {
                return exports.language === 'en';
            }
            else if (exports.language.length >= 3) {
                return exports.language[0] === 'e' && exports.language[1] === 'n' && exports.language[2] === '-';
            }
            else {
                return false;
            }
        }
        Language.isDefaultVariant = isDefaultVariant;
        function isDefault() {
            return exports.language === 'en';
        }
        Language.isDefault = isDefault;
    })(Language = exports.Language || (exports.Language = {}));
    /**
     * The OS locale or the locale specified by --locale. The format of
     * the string is all lower case (e.g. zh-tw for Traditional
     * Chinese). The UI is not necessarily shown in the provided locale.
     */
    exports.locale = _locale;
    /**
     * The translatios that are available through language packs.
     */
    exports.translationsConfigFile = _translationsConfigFile;
    var _globals = (typeof self === 'object' ? self : typeof global === 'object' ? global : {});
    exports.globals = _globals;
    exports.setImmediate = (function defineSetImmediate() {
        if (exports.globals.setImmediate) {
            return exports.globals.setImmediate.bind(exports.globals);
        }
        if (typeof exports.globals.postMessage === 'function' && !exports.globals.importScripts) {
            var pending_1 = [];
            exports.globals.addEventListener('message', function (e) {
                if (e.data && e.data.vscodeSetImmediateId) {
                    for (var i = 0, len = pending_1.length; i < len; i++) {
                        var candidate = pending_1[i];
                        if (candidate.id === e.data.vscodeSetImmediateId) {
                            pending_1.splice(i, 1);
                            candidate.callback();
                            return;
                        }
                    }
                }
            });
            var lastId_1 = 0;
            return function (callback) {
                var myId = ++lastId_1;
                pending_1.push({
                    id: myId,
                    callback: callback
                });
                exports.globals.postMessage({ vscodeSetImmediateId: myId }, '*');
            };
        }
        if (typeof process !== 'undefined' && typeof process.nextTick === 'function') {
            return process.nextTick.bind(process);
        }
        var _promise = Promise.resolve();
        return function (callback) { return _promise.then(callback); };
    })();
    exports.OS = (_isMacintosh ? 2 /* Macintosh */ : (_isWindows ? 1 /* Windows */ : 3 /* Linux */));
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[15/*vs/base/common/strings*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    function isFalsyOrWhitespace(str) {
        if (!str || typeof str !== 'string') {
            return true;
        }
        return str.trim().length === 0;
    }
    exports.isFalsyOrWhitespace = isFalsyOrWhitespace;
    /**
     * @returns the provided number with the given number of preceding zeros.
     */
    function pad(n, l, char) {
        if (char === void 0) { char = '0'; }
        var str = '' + n;
        var r = [str];
        for (var i = str.length; i < l; i++) {
            r.push(char);
        }
        return r.reverse().join('');
    }
    exports.pad = pad;
    var _formatRegexp = /{(\d+)}/g;
    /**
     * Helper to produce a string with a variable number of arguments. Insert variable segments
     * into the string using the {n} notation where N is the index of the argument following the string.
     * @param value string to which formatting is applied
     * @param args replacements for {n}-entries
     */
    function format(value) {
        var args = [];
        for (var _i = 1; _i < arguments.length; _i++) {
            args[_i - 1] = arguments[_i];
        }
        if (args.length === 0) {
            return value;
        }
        return value.replace(_formatRegexp, function (match, group) {
            var idx = parseInt(group, 10);
            return isNaN(idx) || idx < 0 || idx >= args.length ?
                match :
                args[idx];
        });
    }
    exports.format = format;
    /**
     * Converts HTML characters inside the string to use entities instead. Makes the string safe from
     * being used e.g. in HTMLElement.innerHTML.
     */
    function escape(html) {
        return html.replace(/[<>&]/g, function (match) {
            switch (match) {
                case '<': return '&lt;';
                case '>': return '&gt;';
                case '&': return '&amp;';
                default: return match;
            }
        });
    }
    exports.escape = escape;
    /**
     * Escapes regular expression characters in a given string
     */
    function escapeRegExpCharacters(value) {
        return value.replace(/[\\\{\}\*\+\?\|\^\$\.\[\]\(\)]/g, '\\$&');
    }
    exports.escapeRegExpCharacters = escapeRegExpCharacters;
    /**
     * Removes all occurrences of needle from the beginning and end of haystack.
     * @param haystack string to trim
     * @param needle the thing to trim (default is a blank)
     */
    function trim(haystack, needle) {
        if (needle === void 0) { needle = ' '; }
        var trimmed = ltrim(haystack, needle);
        return rtrim(trimmed, needle);
    }
    exports.trim = trim;
    /**
     * Removes all occurrences of needle from the beginning of haystack.
     * @param haystack string to trim
     * @param needle the thing to trim
     */
    function ltrim(haystack, needle) {
        if (!haystack || !needle) {
            return haystack;
        }
        var needleLen = needle.length;
        if (needleLen === 0 || haystack.length === 0) {
            return haystack;
        }
        var offset = 0;
        while (haystack.indexOf(needle, offset) === offset) {
            offset = offset + needleLen;
        }
        return haystack.substring(offset);
    }
    exports.ltrim = ltrim;
    /**
     * Removes all occurrences of needle from the end of haystack.
     * @param haystack string to trim
     * @param needle the thing to trim
     */
    function rtrim(haystack, needle) {
        if (!haystack || !needle) {
            return haystack;
        }
        var needleLen = needle.length, haystackLen = haystack.length;
        if (needleLen === 0 || haystackLen === 0) {
            return haystack;
        }
        var offset = haystackLen, idx = -1;
        while (true) {
            idx = haystack.lastIndexOf(needle, offset - 1);
            if (idx === -1 || idx + needleLen !== offset) {
                break;
            }
            if (idx === 0) {
                return '';
            }
            offset = idx;
        }
        return haystack.substring(0, offset);
    }
    exports.rtrim = rtrim;
    function convertSimple2RegExpPattern(pattern) {
        return pattern.replace(/[\-\\\{\}\+\?\|\^\$\.\,\[\]\(\)\#\s]/g, '\\$&').replace(/[\*]/g, '.*');
    }
    exports.convertSimple2RegExpPattern = convertSimple2RegExpPattern;
    function stripWildcards(pattern) {
        return pattern.replace(/\*/g, '');
    }
    exports.stripWildcards = stripWildcards;
    /**
     * Determines if haystack starts with needle.
     */
    function startsWith(haystack, needle) {
        if (haystack.length < needle.length) {
            return false;
        }
        if (haystack === needle) {
            return true;
        }
        for (var i = 0; i < needle.length; i++) {
            if (haystack[i] !== needle[i]) {
                return false;
            }
        }
        return true;
    }
    exports.startsWith = startsWith;
    /**
     * Determines if haystack ends with needle.
     */
    function endsWith(haystack, needle) {
        var diff = haystack.length - needle.length;
        if (diff > 0) {
            return haystack.indexOf(needle, diff) === diff;
        }
        else if (diff === 0) {
            return haystack === needle;
        }
        else {
            return false;
        }
    }
    exports.endsWith = endsWith;
    function createRegExp(searchString, isRegex, options) {
        if (options === void 0) { options = {}; }
        if (!searchString) {
            throw new Error('Cannot create regex from empty string');
        }
        if (!isRegex) {
            searchString = escapeRegExpCharacters(searchString);
        }
        if (options.wholeWord) {
            if (!/\B/.test(searchString.charAt(0))) {
                searchString = '\\b' + searchString;
            }
            if (!/\B/.test(searchString.charAt(searchString.length - 1))) {
                searchString = searchString + '\\b';
            }
        }
        var modifiers = '';
        if (options.global) {
            modifiers += 'g';
        }
        if (!options.matchCase) {
            modifiers += 'i';
        }
        if (options.multiline) {
            modifiers += 'm';
        }
        if (options.unicode) {
            modifiers += 'u';
        }
        return new RegExp(searchString, modifiers);
    }
    exports.createRegExp = createRegExp;
    function regExpLeadsToEndlessLoop(regexp) {
        // Exit early if it's one of these special cases which are meant to match
        // against an empty string
        if (regexp.source === '^' || regexp.source === '^$' || regexp.source === '$' || regexp.source === '^\\s*$') {
            return false;
        }
        // We check against an empty string. If the regular expression doesn't advance
        // (e.g. ends in an endless loop) it will match an empty string.
        var match = regexp.exec('');
        return !!(match && regexp.lastIndex === 0);
    }
    exports.regExpLeadsToEndlessLoop = regExpLeadsToEndlessLoop;
    function regExpContainsBackreference(regexpValue) {
        return !!regexpValue.match(/([^\\]|^)(\\\\)*\\\d+/);
    }
    exports.regExpContainsBackreference = regExpContainsBackreference;
    function regExpFlags(regexp) {
        return (regexp.global ? 'g' : '')
            + (regexp.ignoreCase ? 'i' : '')
            + (regexp.multiline ? 'm' : '')
            + (regexp.unicode ? 'u' : '');
    }
    exports.regExpFlags = regExpFlags;
    /**
     * Returns first index of the string that is not whitespace.
     * If string is empty or contains only whitespaces, returns -1
     */
    function firstNonWhitespaceIndex(str) {
        for (var i = 0, len = str.length; i < len; i++) {
            var chCode = str.charCodeAt(i);
            if (chCode !== 32 /* Space */ && chCode !== 9 /* Tab */) {
                return i;
            }
        }
        return -1;
    }
    exports.firstNonWhitespaceIndex = firstNonWhitespaceIndex;
    /**
     * Returns the leading whitespace of the string.
     * If the string contains only whitespaces, returns entire string
     */
    function getLeadingWhitespace(str, start, end) {
        if (start === void 0) { start = 0; }
        if (end === void 0) { end = str.length; }
        for (var i = start; i < end; i++) {
            var chCode = str.charCodeAt(i);
            if (chCode !== 32 /* Space */ && chCode !== 9 /* Tab */) {
                return str.substring(start, i);
            }
        }
        return str.substring(start, end);
    }
    exports.getLeadingWhitespace = getLeadingWhitespace;
    /**
     * Returns last index of the string that is not whitespace.
     * If string is empty or contains only whitespaces, returns -1
     */
    function lastNonWhitespaceIndex(str, startIndex) {
        if (startIndex === void 0) { startIndex = str.length - 1; }
        for (var i = startIndex; i >= 0; i--) {
            var chCode = str.charCodeAt(i);
            if (chCode !== 32 /* Space */ && chCode !== 9 /* Tab */) {
                return i;
            }
        }
        return -1;
    }
    exports.lastNonWhitespaceIndex = lastNonWhitespaceIndex;
    function compare(a, b) {
        if (a < b) {
            return -1;
        }
        else if (a > b) {
            return 1;
        }
        else {
            return 0;
        }
    }
    exports.compare = compare;
    function compareIgnoreCase(a, b) {
        var len = Math.min(a.length, b.length);
        for (var i = 0; i < len; i++) {
            var codeA = a.charCodeAt(i);
            var codeB = b.charCodeAt(i);
            if (codeA === codeB) {
                // equal
                continue;
            }
            if (isUpperAsciiLetter(codeA)) {
                codeA += 32;
            }
            if (isUpperAsciiLetter(codeB)) {
                codeB += 32;
            }
            var diff = codeA - codeB;
            if (diff === 0) {
                // equal -> ignoreCase
                continue;
            }
            else if (isLowerAsciiLetter(codeA) && isLowerAsciiLetter(codeB)) {
                //
                return diff;
            }
            else {
                return compare(a.toLowerCase(), b.toLowerCase());
            }
        }
        if (a.length < b.length) {
            return -1;
        }
        else if (a.length > b.length) {
            return 1;
        }
        else {
            return 0;
        }
    }
    exports.compareIgnoreCase = compareIgnoreCase;
    function isLowerAsciiLetter(code) {
        return code >= 97 /* a */ && code <= 122 /* z */;
    }
    exports.isLowerAsciiLetter = isLowerAsciiLetter;
    function isUpperAsciiLetter(code) {
        return code >= 65 /* A */ && code <= 90 /* Z */;
    }
    exports.isUpperAsciiLetter = isUpperAsciiLetter;
    function isAsciiLetter(code) {
        return isLowerAsciiLetter(code) || isUpperAsciiLetter(code);
    }
    function equalsIgnoreCase(a, b) {
        return a.length === b.length && doEqualsIgnoreCase(a, b);
    }
    exports.equalsIgnoreCase = equalsIgnoreCase;
    function doEqualsIgnoreCase(a, b, stopAt) {
        if (stopAt === void 0) { stopAt = a.length; }
        for (var i = 0; i < stopAt; i++) {
            var codeA = a.charCodeAt(i);
            var codeB = b.charCodeAt(i);
            if (codeA === codeB) {
                continue;
            }
            // a-z A-Z
            if (isAsciiLetter(codeA) && isAsciiLetter(codeB)) {
                var diff = Math.abs(codeA - codeB);
                if (diff !== 0 && diff !== 32) {
                    return false;
                }
            }
            // Any other charcode
            else {
                if (String.fromCharCode(codeA).toLowerCase() !== String.fromCharCode(codeB).toLowerCase()) {
                    return false;
                }
            }
        }
        return true;
    }
    function startsWithIgnoreCase(str, candidate) {
        var candidateLength = candidate.length;
        if (candidate.length > str.length) {
            return false;
        }
        return doEqualsIgnoreCase(str, candidate, candidateLength);
    }
    exports.startsWithIgnoreCase = startsWithIgnoreCase;
    /**
     * @returns the length of the common prefix of the two strings.
     */
    function commonPrefixLength(a, b) {
        var i, len = Math.min(a.length, b.length);
        for (i = 0; i < len; i++) {
            if (a.charCodeAt(i) !== b.charCodeAt(i)) {
                return i;
            }
        }
        return len;
    }
    exports.commonPrefixLength = commonPrefixLength;
    /**
     * @returns the length of the common suffix of the two strings.
     */
    function commonSuffixLength(a, b) {
        var i, len = Math.min(a.length, b.length);
        var aLastIndex = a.length - 1;
        var bLastIndex = b.length - 1;
        for (i = 0; i < len; i++) {
            if (a.charCodeAt(aLastIndex - i) !== b.charCodeAt(bLastIndex - i)) {
                return i;
            }
        }
        return len;
    }
    exports.commonSuffixLength = commonSuffixLength;
    function substrEquals(a, aStart, aEnd, b, bStart, bEnd) {
        while (aStart < aEnd && bStart < bEnd) {
            if (a[aStart] !== b[bStart]) {
                return false;
            }
            aStart += 1;
            bStart += 1;
        }
        return true;
    }
    /**
     * Return the overlap between the suffix of `a` and the prefix of `b`.
     * For instance `overlap("foobar", "arr, I'm a pirate") === 2`.
     */
    function overlap(a, b) {
        var aEnd = a.length;
        var bEnd = b.length;
        var aStart = aEnd - bEnd;
        if (aStart === 0) {
            return a === b ? aEnd : 0;
        }
        else if (aStart < 0) {
            bEnd += aStart;
            aStart = 0;
        }
        while (aStart < aEnd && bEnd > 0) {
            if (substrEquals(a, aStart, aEnd, b, 0, bEnd)) {
                return bEnd;
            }
            bEnd -= 1;
            aStart += 1;
        }
        return 0;
    }
    exports.overlap = overlap;
    // --- unicode
    // http://en.wikipedia.org/wiki/Surrogate_pair
    // Returns the code point starting at a specified index in a string
    // Code points U+0000 to U+D7FF and U+E000 to U+FFFF are represented on a single character
    // Code points U+10000 to U+10FFFF are represented on two consecutive characters
    //export function getUnicodePoint(str:string, index:number, len:number):number {
    //	const chrCode = str.charCodeAt(index);
    //	if (0xD800 <= chrCode && chrCode <= 0xDBFF && index + 1 < len) {
    //		const nextChrCode = str.charCodeAt(index + 1);
    //		if (0xDC00 <= nextChrCode && nextChrCode <= 0xDFFF) {
    //			return (chrCode - 0xD800) << 10 + (nextChrCode - 0xDC00) + 0x10000;
    //		}
    //	}
    //	return chrCode;
    //}
    function isHighSurrogate(charCode) {
        return (0xD800 <= charCode && charCode <= 0xDBFF);
    }
    exports.isHighSurrogate = isHighSurrogate;
    function isLowSurrogate(charCode) {
        return (0xDC00 <= charCode && charCode <= 0xDFFF);
    }
    exports.isLowSurrogate = isLowSurrogate;
    /**
     * get the code point that begins at offset `offset`
     */
    function getNextCodePoint(str, len, offset) {
        var charCode = str.charCodeAt(offset);
        if (isHighSurrogate(charCode) && offset + 1 < len) {
            var nextCharCode = str.charCodeAt(offset + 1);
            if (isLowSurrogate(nextCharCode)) {
                return ((charCode - 0xD800) << 10) + (nextCharCode - 0xDC00) + 0x10000;
            }
        }
        return charCode;
    }
    exports.getNextCodePoint = getNextCodePoint;
    /**
     * get the code point that ends right before offset `offset`
     */
    function getPrevCodePoint(str, offset) {
        var charCode = str.charCodeAt(offset - 1);
        if (isLowSurrogate(charCode) && offset > 1) {
            var prevCharCode = str.charCodeAt(offset - 2);
            if (isHighSurrogate(prevCharCode)) {
                return ((prevCharCode - 0xD800) << 10) + (charCode - 0xDC00) + 0x10000;
            }
        }
        return charCode;
    }
    function nextCharLength(str, offset) {
        var graphemeBreakTree = GraphemeBreakTree.getInstance();
        var initialOffset = offset;
        var len = str.length;
        var initialCodePoint = getNextCodePoint(str, len, offset);
        offset += (initialCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
        var graphemeBreakType = graphemeBreakTree.getGraphemeBreakType(initialCodePoint);
        while (offset < len) {
            var nextCodePoint = getNextCodePoint(str, len, offset);
            var nextGraphemeBreakType = graphemeBreakTree.getGraphemeBreakType(nextCodePoint);
            if (breakBetweenGraphemeBreakType(graphemeBreakType, nextGraphemeBreakType)) {
                break;
            }
            offset += (nextCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            graphemeBreakType = nextGraphemeBreakType;
        }
        return (offset - initialOffset);
    }
    exports.nextCharLength = nextCharLength;
    function prevCharLength(str, offset) {
        var graphemeBreakTree = GraphemeBreakTree.getInstance();
        var initialOffset = offset;
        var initialCodePoint = getPrevCodePoint(str, offset);
        offset -= (initialCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
        var graphemeBreakType = graphemeBreakTree.getGraphemeBreakType(initialCodePoint);
        while (offset > 0) {
            var prevCodePoint = getPrevCodePoint(str, offset);
            var prevGraphemeBreakType = graphemeBreakTree.getGraphemeBreakType(prevCodePoint);
            if (breakBetweenGraphemeBreakType(prevGraphemeBreakType, graphemeBreakType)) {
                break;
            }
            offset -= (prevCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            graphemeBreakType = prevGraphemeBreakType;
        }
        return (initialOffset - offset);
    }
    exports.prevCharLength = prevCharLength;
    function _getCharContainingOffset(str, offset) {
        var graphemeBreakTree = GraphemeBreakTree.getInstance();
        var len = str.length;
        var initialOffset = offset;
        var initialCodePoint = getNextCodePoint(str, len, offset);
        var initialGraphemeBreakType = graphemeBreakTree.getGraphemeBreakType(initialCodePoint);
        offset += (initialCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
        // extend to the right
        var graphemeBreakType = initialGraphemeBreakType;
        while (offset < len) {
            var nextCodePoint = getNextCodePoint(str, len, offset);
            var nextGraphemeBreakType = graphemeBreakTree.getGraphemeBreakType(nextCodePoint);
            if (breakBetweenGraphemeBreakType(graphemeBreakType, nextGraphemeBreakType)) {
                break;
            }
            offset += (nextCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            graphemeBreakType = nextGraphemeBreakType;
        }
        var endOffset = offset;
        // extend to the left
        offset = initialOffset;
        graphemeBreakType = initialGraphemeBreakType;
        while (offset > 0) {
            var prevCodePoint = getPrevCodePoint(str, offset);
            var prevGraphemeBreakType = graphemeBreakTree.getGraphemeBreakType(prevCodePoint);
            if (breakBetweenGraphemeBreakType(prevGraphemeBreakType, graphemeBreakType)) {
                break;
            }
            offset -= (prevCodePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            graphemeBreakType = prevGraphemeBreakType;
        }
        return [offset, endOffset];
    }
    function getCharContainingOffset(str, offset) {
        if (offset > 0 && isLowSurrogate(str.charCodeAt(offset))) {
            return _getCharContainingOffset(str, offset - 1);
        }
        return _getCharContainingOffset(str, offset);
    }
    exports.getCharContainingOffset = getCharContainingOffset;
    /**
     * A manual encoding of `str` to UTF8.
     * Use only in environments which do not offer native conversion methods!
     */
    function encodeUTF8(str) {
        var strLen = str.length;
        // See https://en.wikipedia.org/wiki/UTF-8
        // first loop to establish needed buffer size
        var neededSize = 0;
        var strOffset = 0;
        while (strOffset < strLen) {
            var codePoint = getNextCodePoint(str, strLen, strOffset);
            strOffset += (codePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            if (codePoint < 0x0080) {
                neededSize += 1;
            }
            else if (codePoint < 0x0800) {
                neededSize += 2;
            }
            else if (codePoint < 0x10000) {
                neededSize += 3;
            }
            else {
                neededSize += 4;
            }
        }
        // second loop to actually encode
        var arr = new Uint8Array(neededSize);
        strOffset = 0;
        var arrOffset = 0;
        while (strOffset < strLen) {
            var codePoint = getNextCodePoint(str, strLen, strOffset);
            strOffset += (codePoint >= 65536 /* UNICODE_SUPPLEMENTARY_PLANE_BEGIN */ ? 2 : 1);
            if (codePoint < 0x0080) {
                arr[arrOffset++] = codePoint;
            }
            else if (codePoint < 0x0800) {
                arr[arrOffset++] = 192 | ((codePoint & 1984) >>> 6);
                arr[arrOffset++] = 128 | ((codePoint & 63) >>> 0);
            }
            else if (codePoint < 0x10000) {
                arr[arrOffset++] = 224 | ((codePoint & 61440) >>> 12);
                arr[arrOffset++] = 128 | ((codePoint & 4032) >>> 6);
                arr[arrOffset++] = 128 | ((codePoint & 63) >>> 0);
            }
            else {
                arr[arrOffset++] = 240 | ((codePoint & 1835008) >>> 18);
                arr[arrOffset++] = 128 | ((codePoint & 258048) >>> 12);
                arr[arrOffset++] = 128 | ((codePoint & 4032) >>> 6);
                arr[arrOffset++] = 128 | ((codePoint & 63) >>> 0);
            }
        }
        return arr;
    }
    exports.encodeUTF8 = encodeUTF8;
    /**
     * A manual decoding of a UTF8 string.
     * Use only in environments which do not offer native conversion methods!
     */
    function decodeUTF8(buffer) {
        // https://en.wikipedia.org/wiki/UTF-8
        var len = buffer.byteLength;
        var result = [];
        var offset = 0;
        while (offset < len) {
            var v0 = buffer[offset];
            var codePoint = void 0;
            if (v0 >= 240 && offset + 3 < len) {
                // 4 bytes
                codePoint = ((((buffer[offset++] & 7) << 18) >>> 0)
                    | (((buffer[offset++] & 63) << 12) >>> 0)
                    | (((buffer[offset++] & 63) << 6) >>> 0)
                    | (((buffer[offset++] & 63) << 0) >>> 0));
            }
            else if (v0 >= 224 && offset + 2 < len) {
                // 3 bytes
                codePoint = ((((buffer[offset++] & 15) << 12) >>> 0)
                    | (((buffer[offset++] & 63) << 6) >>> 0)
                    | (((buffer[offset++] & 63) << 0) >>> 0));
            }
            else if (v0 >= 192 && offset + 1 < len) {
                // 2 bytes
                codePoint = ((((buffer[offset++] & 31) << 6) >>> 0)
                    | (((buffer[offset++] & 63) << 0) >>> 0));
            }
            else {
                // 1 byte
                codePoint = buffer[offset++];
            }
            if ((codePoint >= 0 && codePoint <= 0xD7FF) || (codePoint >= 0xE000 && codePoint <= 0xFFFF)) {
                // Basic Multilingual Plane
                result.push(String.fromCharCode(codePoint));
            }
            else if (codePoint >= 0x010000 && codePoint <= 0x10FFFF) {
                // Supplementary Planes
                var uPrime = codePoint - 0x10000;
                var w1 = 0xD800 + ((uPrime & 1047552) >>> 10);
                var w2 = 0xDC00 + ((uPrime & 1023) >>> 0);
                result.push(String.fromCharCode(w1));
                result.push(String.fromCharCode(w2));
            }
            else {
                // illegal code point
                result.push(String.fromCharCode(0xFFFD));
            }
        }
        return result.join('');
    }
    exports.decodeUTF8 = decodeUTF8;
    /**
     * Generated using https://github.com/alexandrudima/unicode-utils/blob/master/generate-rtl-test.js
     */
    var CONTAINS_RTL = /(?:[\u05BE\u05C0\u05C3\u05C6\u05D0-\u05F4\u0608\u060B\u060D\u061B-\u064A\u066D-\u066F\u0671-\u06D5\u06E5\u06E6\u06EE\u06EF\u06FA-\u0710\u0712-\u072F\u074D-\u07A5\u07B1-\u07EA\u07F4\u07F5\u07FA-\u0815\u081A\u0824\u0828\u0830-\u0858\u085E-\u08BD\u200F\uFB1D\uFB1F-\uFB28\uFB2A-\uFD3D\uFD50-\uFDFC\uFE70-\uFEFC]|\uD802[\uDC00-\uDD1B\uDD20-\uDE00\uDE10-\uDE33\uDE40-\uDEE4\uDEEB-\uDF35\uDF40-\uDFFF]|\uD803[\uDC00-\uDCFF]|\uD83A[\uDC00-\uDCCF\uDD00-\uDD43\uDD50-\uDFFF]|\uD83B[\uDC00-\uDEBB])/;
    /**
     * Returns true if `str` contains any Unicode character that is classified as "R" or "AL".
     */
    function containsRTL(str) {
        return CONTAINS_RTL.test(str);
    }
    exports.containsRTL = containsRTL;
    /**
     * Generated using https://github.com/alexandrudima/unicode-utils/blob/master/generate-emoji-test.js
     */
    var CONTAINS_EMOJI = /(?:[\u231A\u231B\u23F0\u23F3\u2600-\u27BF\u2B50\u2B55]|\uD83C[\uDDE6-\uDDFF\uDF00-\uDFFF]|\uD83D[\uDC00-\uDE4F\uDE80-\uDEFC\uDFE0-\uDFEB]|\uD83E[\uDD00-\uDDFF\uDE70-\uDE73\uDE78-\uDE82\uDE90-\uDE95])/;
    function containsEmoji(str) {
        return CONTAINS_EMOJI.test(str);
    }
    exports.containsEmoji = containsEmoji;
    var IS_BASIC_ASCII = /^[\t\n\r\x20-\x7E]*$/;
    /**
     * Returns true if `str` contains only basic ASCII characters in the range 32 - 126 (including 32 and 126) or \n, \r, \t
     */
    function isBasicASCII(str) {
        return IS_BASIC_ASCII.test(str);
    }
    exports.isBasicASCII = isBasicASCII;
    function containsFullWidthCharacter(str) {
        for (var i = 0, len = str.length; i < len; i++) {
            if (isFullWidthCharacter(str.charCodeAt(i))) {
                return true;
            }
        }
        return false;
    }
    exports.containsFullWidthCharacter = containsFullWidthCharacter;
    function isFullWidthCharacter(charCode) {
        // Do a cheap trick to better support wrapping of wide characters, treat them as 2 columns
        // http://jrgraphix.net/research/unicode_blocks.php
        //          2E80 — 2EFF   CJK Radicals Supplement
        //          2F00 — 2FDF   Kangxi Radicals
        //          2FF0 — 2FFF   Ideographic Description Characters
        //          3000 — 303F   CJK Symbols and Punctuation
        //          3040 — 309F   Hiragana
        //          30A0 — 30FF   Katakana
        //          3100 — 312F   Bopomofo
        //          3130 — 318F   Hangul Compatibility Jamo
        //          3190 — 319F   Kanbun
        //          31A0 — 31BF   Bopomofo Extended
        //          31F0 — 31FF   Katakana Phonetic Extensions
        //          3200 — 32FF   Enclosed CJK Letters and Months
        //          3300 — 33FF   CJK Compatibility
        //          3400 — 4DBF   CJK Unified Ideographs Extension A
        //          4DC0 — 4DFF   Yijing Hexagram Symbols
        //          4E00 — 9FFF   CJK Unified Ideographs
        //          A000 — A48F   Yi Syllables
        //          A490 — A4CF   Yi Radicals
        //          AC00 — D7AF   Hangul Syllables
        // [IGNORE] D800 — DB7F   High Surrogates
        // [IGNORE] DB80 — DBFF   High Private Use Surrogates
        // [IGNORE] DC00 — DFFF   Low Surrogates
        // [IGNORE] E000 — F8FF   Private Use Area
        //          F900 — FAFF   CJK Compatibility Ideographs
        // [IGNORE] FB00 — FB4F   Alphabetic Presentation Forms
        // [IGNORE] FB50 — FDFF   Arabic Presentation Forms-A
        // [IGNORE] FE00 — FE0F   Variation Selectors
        // [IGNORE] FE20 — FE2F   Combining Half Marks
        // [IGNORE] FE30 — FE4F   CJK Compatibility Forms
        // [IGNORE] FE50 — FE6F   Small Form Variants
        // [IGNORE] FE70 — FEFF   Arabic Presentation Forms-B
        //          FF00 — FFEF   Halfwidth and Fullwidth Forms
        //               [https://en.wikipedia.org/wiki/Halfwidth_and_fullwidth_forms]
        //               of which FF01 - FF5E fullwidth ASCII of 21 to 7E
        // [IGNORE]    and FF65 - FFDC halfwidth of Katakana and Hangul
        // [IGNORE] FFF0 — FFFF   Specials
        charCode = +charCode; // @perf
        return ((charCode >= 0x2E80 && charCode <= 0xD7AF)
            || (charCode >= 0xF900 && charCode <= 0xFAFF)
            || (charCode >= 0xFF01 && charCode <= 0xFF5E));
    }
    exports.isFullWidthCharacter = isFullWidthCharacter;
    /**
     * A fast function (therefore imprecise) to check if code points are emojis.
     * Generated using https://github.com/alexandrudima/unicode-utils/blob/master/generate-emoji-test.js
     */
    function isEmojiImprecise(x) {
        return ((x >= 0x1F1E6 && x <= 0x1F1FF) || (x >= 9728 && x <= 10175) || (x >= 127744 && x <= 128591)
            || (x >= 128640 && x <= 128764) || (x >= 128992 && x <= 129003) || (x >= 129280 && x <= 129535)
            || (x >= 129648 && x <= 129651) || (x >= 129656 && x <= 129666) || (x >= 129680 && x <= 129685));
    }
    exports.isEmojiImprecise = isEmojiImprecise;
    /**
     * Given a string and a max length returns a shorted version. Shorting
     * happens at favorable positions - such as whitespace or punctuation characters.
     */
    function lcut(text, n) {
        if (text.length < n) {
            return text;
        }
        var re = /\b/g;
        var i = 0;
        while (re.test(text)) {
            if (text.length - re.lastIndex < n) {
                break;
            }
            i = re.lastIndex;
            re.lastIndex += 1;
        }
        return text.substring(i).replace(/^\s/, '');
    }
    exports.lcut = lcut;
    // Escape codes
    // http://en.wikipedia.org/wiki/ANSI_escape_code
    var EL = /\x1B\x5B[12]?K/g; // Erase in line
    var COLOR_START = /\x1b\[\d+m/g; // Color
    var COLOR_END = /\x1b\[0?m/g; // Color
    function removeAnsiEscapeCodes(str) {
        if (str) {
            str = str.replace(EL, '');
            str = str.replace(COLOR_START, '');
            str = str.replace(COLOR_END, '');
        }
        return str;
    }
    exports.removeAnsiEscapeCodes = removeAnsiEscapeCodes;
    exports.removeAccents = (function () {
        if (typeof String.prototype.normalize !== 'function') {
            // ☹️ no ES6 features...
            return function (str) { return str; };
        }
        else {
            // transform into NFD form and remove accents
            // see: https://stackoverflow.com/questions/990904/remove-accents-diacritics-in-a-string-in-javascript/37511463#37511463
            var regex_1 = /[\u0300-\u036f]/g;
            return function (str) {
                return str.normalize('NFD').replace(regex_1, '');
            };
        }
    })();
    // -- UTF-8 BOM
    exports.UTF8_BOM_CHARACTER = String.fromCharCode(65279 /* UTF8_BOM */);
    function startsWithUTF8BOM(str) {
        return !!(str && str.length > 0 && str.charCodeAt(0) === 65279 /* UTF8_BOM */);
    }
    exports.startsWithUTF8BOM = startsWithUTF8BOM;
    function stripUTF8BOM(str) {
        return startsWithUTF8BOM(str) ? str.substr(1) : str;
    }
    exports.stripUTF8BOM = stripUTF8BOM;
    function safeBtoa(str) {
        return btoa(encodeURIComponent(str)); // we use encodeURIComponent because btoa fails for non Latin 1 values
    }
    exports.safeBtoa = safeBtoa;
    function repeat(s, count) {
        var result = '';
        for (var i = 0; i < count; i++) {
            result += s;
        }
        return result;
    }
    exports.repeat = repeat;
    /**
     * Checks if the characters of the provided query string are included in the
     * target string. The characters do not have to be contiguous within the string.
     */
    function fuzzyContains(target, query) {
        if (!target || !query) {
            return false; // return early if target or query are undefined
        }
        if (target.length < query.length) {
            return false; // impossible for query to be contained in target
        }
        var queryLen = query.length;
        var targetLower = target.toLowerCase();
        var index = 0;
        var lastIndexOf = -1;
        while (index < queryLen) {
            var indexOf = targetLower.indexOf(query[index], lastIndexOf + 1);
            if (indexOf < 0) {
                return false;
            }
            lastIndexOf = indexOf;
            index++;
        }
        return true;
    }
    exports.fuzzyContains = fuzzyContains;
    function containsUppercaseCharacter(target, ignoreEscapedChars) {
        if (ignoreEscapedChars === void 0) { ignoreEscapedChars = false; }
        if (!target) {
            return false;
        }
        if (ignoreEscapedChars) {
            target = target.replace(/\\./g, '');
        }
        return target.toLowerCase() !== target;
    }
    exports.containsUppercaseCharacter = containsUppercaseCharacter;
    function uppercaseFirstLetter(str) {
        return str.charAt(0).toUpperCase() + str.slice(1);
    }
    exports.uppercaseFirstLetter = uppercaseFirstLetter;
    function getNLines(str, n) {
        if (n === void 0) { n = 1; }
        if (n === 0) {
            return '';
        }
        var idx = -1;
        do {
            idx = str.indexOf('\n', idx + 1);
            n--;
        } while (n > 0 && idx >= 0);
        return idx >= 0 ?
            str.substr(0, idx) :
            str;
    }
    exports.getNLines = getNLines;
    /**
     * Produces 'a'-'z', followed by 'A'-'Z'... followed by 'a'-'z', etc.
     */
    function singleLetterHash(n) {
        var LETTERS_CNT = (90 /* Z */ - 65 /* A */ + 1);
        n = n % (2 * LETTERS_CNT);
        if (n < LETTERS_CNT) {
            return String.fromCharCode(97 /* a */ + n);
        }
        return String.fromCharCode(65 /* A */ + n - LETTERS_CNT);
    }
    exports.singleLetterHash = singleLetterHash;
    //#region Unicode Grapheme Break
    function getGraphemeBreakType(codePoint) {
        var graphemeBreakTree = GraphemeBreakTree.getInstance();
        return graphemeBreakTree.getGraphemeBreakType(codePoint);
    }
    exports.getGraphemeBreakType = getGraphemeBreakType;
    function breakBetweenGraphemeBreakType(breakTypeA, breakTypeB) {
        // http://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundary_Rules
        // !!! Let's make the common case a bit faster
        if (breakTypeA === 0 /* Other */) {
            // see https://www.unicode.org/Public/13.0.0/ucd/auxiliary/GraphemeBreakTest-13.0.0d10.html#table
            return (breakTypeB !== 5 /* Extend */ && breakTypeB !== 7 /* SpacingMark */);
        }
        // Do not break between a CR and LF. Otherwise, break before and after controls.
        // GB3                                        CR × LF
        // GB4                       (Control | CR | LF) ÷
        // GB5                                           ÷ (Control | CR | LF)
        if (breakTypeA === 2 /* CR */) {
            if (breakTypeB === 3 /* LF */) {
                return false; // GB3
            }
        }
        if (breakTypeA === 4 /* Control */ || breakTypeA === 2 /* CR */ || breakTypeA === 3 /* LF */) {
            return true; // GB4
        }
        if (breakTypeB === 4 /* Control */ || breakTypeB === 2 /* CR */ || breakTypeB === 3 /* LF */) {
            return true; // GB5
        }
        // Do not break Hangul syllable sequences.
        // GB6                                         L × (L | V | LV | LVT)
        // GB7                                  (LV | V) × (V | T)
        // GB8                                 (LVT | T) × T
        if (breakTypeA === 8 /* L */) {
            if (breakTypeB === 8 /* L */ || breakTypeB === 9 /* V */ || breakTypeB === 11 /* LV */ || breakTypeB === 12 /* LVT */) {
                return false; // GB6
            }
        }
        if (breakTypeA === 11 /* LV */ || breakTypeA === 9 /* V */) {
            if (breakTypeB === 9 /* V */ || breakTypeB === 10 /* T */) {
                return false; // GB7
            }
        }
        if (breakTypeA === 12 /* LVT */ || breakTypeA === 10 /* T */) {
            if (breakTypeB === 10 /* T */) {
                return false; // GB8
            }
        }
        // Do not break before extending characters or ZWJ.
        // GB9                                           × (Extend | ZWJ)
        if (breakTypeB === 5 /* Extend */ || breakTypeB === 13 /* ZWJ */) {
            return false; // GB9
        }
        // The GB9a and GB9b rules only apply to extended grapheme clusters:
        // Do not break before SpacingMarks, or after Prepend characters.
        // GB9a                                          × SpacingMark
        // GB9b                                  Prepend ×
        if (breakTypeB === 7 /* SpacingMark */) {
            return false; // GB9a
        }
        if (breakTypeA === 1 /* Prepend */) {
            return false; // GB9b
        }
        // Do not break within emoji modifier sequences or emoji zwj sequences.
        // GB11    \p{Extended_Pictographic} Extend* ZWJ × \p{Extended_Pictographic}
        if (breakTypeA === 13 /* ZWJ */ && breakTypeB === 14 /* Extended_Pictographic */) {
            // Note: we are not implementing the rule entirely here to avoid introducing states
            return false; // GB11
        }
        // GB12                          sot (RI RI)* RI × RI
        // GB13                        [^RI] (RI RI)* RI × RI
        if (breakTypeA === 6 /* Regional_Indicator */ && breakTypeB === 6 /* Regional_Indicator */) {
            // Note: we are not implementing the rule entirely here to avoid introducing states
            return false; // GB12 & GB13
        }
        // GB999                                     Any ÷ Any
        return true;
    }
    exports.breakBetweenGraphemeBreakType = breakBetweenGraphemeBreakType;
    var GraphemeBreakTree = /** @class */ (function () {
        function GraphemeBreakTree() {
            this._data = getGraphemeBreakRawData();
        }
        GraphemeBreakTree.getInstance = function () {
            if (!GraphemeBreakTree._INSTANCE) {
                GraphemeBreakTree._INSTANCE = new GraphemeBreakTree();
            }
            return GraphemeBreakTree._INSTANCE;
        };
        GraphemeBreakTree.prototype.getGraphemeBreakType = function (codePoint) {
            // !!! Let's make 7bit ASCII a bit faster: 0..31
            if (codePoint < 32) {
                if (codePoint === 10 /* LineFeed */) {
                    return 3 /* LF */;
                }
                if (codePoint === 13 /* CarriageReturn */) {
                    return 2 /* CR */;
                }
                return 4 /* Control */;
            }
            // !!! Let's make 7bit ASCII a bit faster: 32..126
            if (codePoint < 127) {
                return 0 /* Other */;
            }
            var data = this._data;
            var nodeCount = data.length / 3;
            var nodeIndex = 1;
            while (nodeIndex <= nodeCount) {
                if (codePoint < data[3 * nodeIndex]) {
                    // go left
                    nodeIndex = 2 * nodeIndex;
                }
                else if (codePoint > data[3 * nodeIndex + 1]) {
                    // go right
                    nodeIndex = 2 * nodeIndex + 1;
                }
                else {
                    // hit
                    return data[3 * nodeIndex + 2];
                }
            }
            return 0 /* Other */;
        };
        GraphemeBreakTree._INSTANCE = null;
        return GraphemeBreakTree;
    }());
    function getGraphemeBreakRawData() {
        // generated using https://github.com/alexandrudima/unicode-utils/blob/master/generate-grapheme-break.js
        return JSON.parse('[0,0,0,51592,51592,11,44424,44424,11,72251,72254,5,7150,7150,7,48008,48008,11,55176,55176,11,128420,128420,14,3276,3277,5,9979,9980,14,46216,46216,11,49800,49800,11,53384,53384,11,70726,70726,5,122915,122916,5,129320,129327,14,2558,2558,5,5906,5908,5,9762,9763,14,43360,43388,8,45320,45320,11,47112,47112,11,48904,48904,11,50696,50696,11,52488,52488,11,54280,54280,11,70082,70083,1,71350,71350,7,73111,73111,5,127892,127893,14,128726,128727,14,129473,129474,14,2027,2035,5,2901,2902,5,3784,3789,5,6754,6754,5,8418,8420,5,9877,9877,14,11088,11088,14,44008,44008,5,44872,44872,11,45768,45768,11,46664,46664,11,47560,47560,11,48456,48456,11,49352,49352,11,50248,50248,11,51144,51144,11,52040,52040,11,52936,52936,11,53832,53832,11,54728,54728,11,69811,69814,5,70459,70460,5,71096,71099,7,71998,71998,5,72874,72880,5,119149,119149,7,127374,127374,14,128335,128335,14,128482,128482,14,128765,128767,14,129399,129400,14,129680,129685,14,1476,1477,5,2377,2380,7,2759,2760,5,3137,3140,7,3458,3459,7,4153,4154,5,6432,6434,5,6978,6978,5,7675,7679,5,9723,9726,14,9823,9823,14,9919,9923,14,10035,10036,14,42736,42737,5,43596,43596,5,44200,44200,11,44648,44648,11,45096,45096,11,45544,45544,11,45992,45992,11,46440,46440,11,46888,46888,11,47336,47336,11,47784,47784,11,48232,48232,11,48680,48680,11,49128,49128,11,49576,49576,11,50024,50024,11,50472,50472,11,50920,50920,11,51368,51368,11,51816,51816,11,52264,52264,11,52712,52712,11,53160,53160,11,53608,53608,11,54056,54056,11,54504,54504,11,54952,54952,11,68108,68111,5,69933,69940,5,70197,70197,7,70498,70499,7,70845,70845,5,71229,71229,5,71727,71735,5,72154,72155,5,72344,72345,5,73023,73029,5,94095,94098,5,121403,121452,5,126981,127182,14,127538,127546,14,127990,127990,14,128391,128391,14,128445,128449,14,128500,128505,14,128752,128752,14,129160,129167,14,129356,129356,14,129432,129442,14,129648,129651,14,129751,131069,14,173,173,4,1757,1757,1,2274,2274,1,2494,2494,5,2641,2641,5,2876,2876,5,3014,3016,7,3262,3262,7,3393,3396,5,3570,3571,7,3968,3972,5,4228,4228,7,6086,6086,5,6679,6680,5,6912,6915,5,7080,7081,5,7380,7392,5,8252,8252,14,9096,9096,14,9748,9749,14,9784,9786,14,9833,9850,14,9890,9894,14,9938,9938,14,9999,9999,14,10085,10087,14,12349,12349,14,43136,43137,7,43454,43456,7,43755,43755,7,44088,44088,11,44312,44312,11,44536,44536,11,44760,44760,11,44984,44984,11,45208,45208,11,45432,45432,11,45656,45656,11,45880,45880,11,46104,46104,11,46328,46328,11,46552,46552,11,46776,46776,11,47000,47000,11,47224,47224,11,47448,47448,11,47672,47672,11,47896,47896,11,48120,48120,11,48344,48344,11,48568,48568,11,48792,48792,11,49016,49016,11,49240,49240,11,49464,49464,11,49688,49688,11,49912,49912,11,50136,50136,11,50360,50360,11,50584,50584,11,50808,50808,11,51032,51032,11,51256,51256,11,51480,51480,11,51704,51704,11,51928,51928,11,52152,52152,11,52376,52376,11,52600,52600,11,52824,52824,11,53048,53048,11,53272,53272,11,53496,53496,11,53720,53720,11,53944,53944,11,54168,54168,11,54392,54392,11,54616,54616,11,54840,54840,11,55064,55064,11,65438,65439,5,69633,69633,5,69837,69837,1,70018,70018,7,70188,70190,7,70368,70370,7,70465,70468,7,70712,70719,5,70835,70840,5,70850,70851,5,71132,71133,5,71340,71340,7,71458,71461,5,71985,71989,7,72002,72002,7,72193,72202,5,72281,72283,5,72766,72766,7,72885,72886,5,73104,73105,5,92912,92916,5,113824,113827,4,119173,119179,5,121505,121519,5,125136,125142,5,127279,127279,14,127489,127490,14,127570,127743,14,127900,127901,14,128254,128254,14,128369,128370,14,128400,128400,14,128425,128432,14,128468,128475,14,128489,128494,14,128715,128720,14,128745,128745,14,128759,128760,14,129004,129023,14,129296,129304,14,129340,129342,14,129388,129392,14,129404,129407,14,129454,129455,14,129485,129487,14,129659,129663,14,129719,129727,14,917536,917631,5,13,13,2,1160,1161,5,1564,1564,4,1807,1807,1,2085,2087,5,2363,2363,7,2402,2403,5,2507,2508,7,2622,2624,7,2691,2691,7,2786,2787,5,2881,2884,5,3006,3006,5,3072,3072,5,3170,3171,5,3267,3268,7,3330,3331,7,3406,3406,1,3538,3540,5,3655,3662,5,3897,3897,5,4038,4038,5,4184,4185,5,4352,4447,8,6068,6069,5,6155,6157,5,6448,6449,7,6742,6742,5,6783,6783,5,6966,6970,5,7042,7042,7,7143,7143,7,7212,7219,5,7412,7412,5,8206,8207,4,8294,8303,4,8596,8601,14,9410,9410,14,9742,9742,14,9757,9757,14,9770,9770,14,9794,9794,14,9828,9828,14,9855,9855,14,9882,9882,14,9900,9903,14,9929,9933,14,9963,9967,14,9987,9988,14,10006,10006,14,10062,10062,14,10175,10175,14,11744,11775,5,42607,42607,5,43043,43044,7,43263,43263,5,43444,43445,7,43569,43570,5,43698,43700,5,43766,43766,5,44032,44032,11,44144,44144,11,44256,44256,11,44368,44368,11,44480,44480,11,44592,44592,11,44704,44704,11,44816,44816,11,44928,44928,11,45040,45040,11,45152,45152,11,45264,45264,11,45376,45376,11,45488,45488,11,45600,45600,11,45712,45712,11,45824,45824,11,45936,45936,11,46048,46048,11,46160,46160,11,46272,46272,11,46384,46384,11,46496,46496,11,46608,46608,11,46720,46720,11,46832,46832,11,46944,46944,11,47056,47056,11,47168,47168,11,47280,47280,11,47392,47392,11,47504,47504,11,47616,47616,11,47728,47728,11,47840,47840,11,47952,47952,11,48064,48064,11,48176,48176,11,48288,48288,11,48400,48400,11,48512,48512,11,48624,48624,11,48736,48736,11,48848,48848,11,48960,48960,11,49072,49072,11,49184,49184,11,49296,49296,11,49408,49408,11,49520,49520,11,49632,49632,11,49744,49744,11,49856,49856,11,49968,49968,11,50080,50080,11,50192,50192,11,50304,50304,11,50416,50416,11,50528,50528,11,50640,50640,11,50752,50752,11,50864,50864,11,50976,50976,11,51088,51088,11,51200,51200,11,51312,51312,11,51424,51424,11,51536,51536,11,51648,51648,11,51760,51760,11,51872,51872,11,51984,51984,11,52096,52096,11,52208,52208,11,52320,52320,11,52432,52432,11,52544,52544,11,52656,52656,11,52768,52768,11,52880,52880,11,52992,52992,11,53104,53104,11,53216,53216,11,53328,53328,11,53440,53440,11,53552,53552,11,53664,53664,11,53776,53776,11,53888,53888,11,54000,54000,11,54112,54112,11,54224,54224,11,54336,54336,11,54448,54448,11,54560,54560,11,54672,54672,11,54784,54784,11,54896,54896,11,55008,55008,11,55120,55120,11,64286,64286,5,66272,66272,5,68900,68903,5,69762,69762,7,69817,69818,5,69927,69931,5,70003,70003,5,70070,70078,5,70094,70094,7,70194,70195,7,70206,70206,5,70400,70401,5,70463,70463,7,70475,70477,7,70512,70516,5,70722,70724,5,70832,70832,5,70842,70842,5,70847,70848,5,71088,71089,7,71102,71102,7,71219,71226,5,71231,71232,5,71342,71343,7,71453,71455,5,71463,71467,5,71737,71738,5,71995,71996,5,72000,72000,7,72145,72147,7,72160,72160,5,72249,72249,7,72273,72278,5,72330,72342,5,72752,72758,5,72850,72871,5,72882,72883,5,73018,73018,5,73031,73031,5,73109,73109,5,73461,73462,7,94031,94031,5,94192,94193,7,119142,119142,7,119155,119162,4,119362,119364,5,121476,121476,5,122888,122904,5,123184,123190,5,126976,126979,14,127184,127231,14,127344,127345,14,127405,127461,14,127514,127514,14,127561,127567,14,127778,127779,14,127896,127896,14,127985,127986,14,127995,127999,5,128326,128328,14,128360,128366,14,128378,128378,14,128394,128397,14,128405,128406,14,128422,128423,14,128435,128443,14,128453,128464,14,128479,128480,14,128484,128487,14,128496,128498,14,128640,128709,14,128723,128724,14,128736,128741,14,128747,128748,14,128755,128755,14,128762,128762,14,128981,128991,14,129096,129103,14,129292,129292,14,129311,129311,14,129329,129330,14,129344,129349,14,129360,129374,14,129394,129394,14,129402,129402,14,129413,129425,14,129445,129450,14,129466,129471,14,129483,129483,14,129511,129535,14,129653,129655,14,129667,129670,14,129705,129711,14,129731,129743,14,917505,917505,4,917760,917999,5,10,10,3,127,159,4,768,879,5,1471,1471,5,1536,1541,1,1648,1648,5,1767,1768,5,1840,1866,5,2070,2073,5,2137,2139,5,2307,2307,7,2366,2368,7,2382,2383,7,2434,2435,7,2497,2500,5,2519,2519,5,2563,2563,7,2631,2632,5,2677,2677,5,2750,2752,7,2763,2764,7,2817,2817,5,2879,2879,5,2891,2892,7,2914,2915,5,3008,3008,5,3021,3021,5,3076,3076,5,3146,3149,5,3202,3203,7,3264,3265,7,3271,3272,7,3298,3299,5,3390,3390,5,3402,3404,7,3426,3427,5,3535,3535,5,3544,3550,7,3635,3635,7,3763,3763,7,3893,3893,5,3953,3966,5,3981,3991,5,4145,4145,7,4157,4158,5,4209,4212,5,4237,4237,5,4520,4607,10,5970,5971,5,6071,6077,5,6089,6099,5,6277,6278,5,6439,6440,5,6451,6456,7,6683,6683,5,6744,6750,5,6765,6770,7,6846,6846,5,6964,6964,5,6972,6972,5,7019,7027,5,7074,7077,5,7083,7085,5,7146,7148,7,7154,7155,7,7222,7223,5,7394,7400,5,7416,7417,5,8204,8204,5,8233,8233,4,8288,8292,4,8413,8416,5,8482,8482,14,8986,8987,14,9193,9203,14,9654,9654,14,9733,9733,14,9745,9745,14,9752,9752,14,9760,9760,14,9766,9766,14,9774,9775,14,9792,9792,14,9800,9811,14,9825,9826,14,9831,9831,14,9852,9853,14,9872,9873,14,9880,9880,14,9885,9887,14,9896,9897,14,9906,9916,14,9926,9927,14,9936,9936,14,9941,9960,14,9974,9974,14,9982,9985,14,9992,9997,14,10002,10002,14,10017,10017,14,10055,10055,14,10071,10071,14,10145,10145,14,11013,11015,14,11503,11505,5,12334,12335,5,12951,12951,14,42612,42621,5,43014,43014,5,43047,43047,7,43204,43205,5,43335,43345,5,43395,43395,7,43450,43451,7,43561,43566,5,43573,43574,5,43644,43644,5,43710,43711,5,43758,43759,7,44005,44005,5,44012,44012,7,44060,44060,11,44116,44116,11,44172,44172,11,44228,44228,11,44284,44284,11,44340,44340,11,44396,44396,11,44452,44452,11,44508,44508,11,44564,44564,11,44620,44620,11,44676,44676,11,44732,44732,11,44788,44788,11,44844,44844,11,44900,44900,11,44956,44956,11,45012,45012,11,45068,45068,11,45124,45124,11,45180,45180,11,45236,45236,11,45292,45292,11,45348,45348,11,45404,45404,11,45460,45460,11,45516,45516,11,45572,45572,11,45628,45628,11,45684,45684,11,45740,45740,11,45796,45796,11,45852,45852,11,45908,45908,11,45964,45964,11,46020,46020,11,46076,46076,11,46132,46132,11,46188,46188,11,46244,46244,11,46300,46300,11,46356,46356,11,46412,46412,11,46468,46468,11,46524,46524,11,46580,46580,11,46636,46636,11,46692,46692,11,46748,46748,11,46804,46804,11,46860,46860,11,46916,46916,11,46972,46972,11,47028,47028,11,47084,47084,11,47140,47140,11,47196,47196,11,47252,47252,11,47308,47308,11,47364,47364,11,47420,47420,11,47476,47476,11,47532,47532,11,47588,47588,11,47644,47644,11,47700,47700,11,47756,47756,11,47812,47812,11,47868,47868,11,47924,47924,11,47980,47980,11,48036,48036,11,48092,48092,11,48148,48148,11,48204,48204,11,48260,48260,11,48316,48316,11,48372,48372,11,48428,48428,11,48484,48484,11,48540,48540,11,48596,48596,11,48652,48652,11,48708,48708,11,48764,48764,11,48820,48820,11,48876,48876,11,48932,48932,11,48988,48988,11,49044,49044,11,49100,49100,11,49156,49156,11,49212,49212,11,49268,49268,11,49324,49324,11,49380,49380,11,49436,49436,11,49492,49492,11,49548,49548,11,49604,49604,11,49660,49660,11,49716,49716,11,49772,49772,11,49828,49828,11,49884,49884,11,49940,49940,11,49996,49996,11,50052,50052,11,50108,50108,11,50164,50164,11,50220,50220,11,50276,50276,11,50332,50332,11,50388,50388,11,50444,50444,11,50500,50500,11,50556,50556,11,50612,50612,11,50668,50668,11,50724,50724,11,50780,50780,11,50836,50836,11,50892,50892,11,50948,50948,11,51004,51004,11,51060,51060,11,51116,51116,11,51172,51172,11,51228,51228,11,51284,51284,11,51340,51340,11,51396,51396,11,51452,51452,11,51508,51508,11,51564,51564,11,51620,51620,11,51676,51676,11,51732,51732,11,51788,51788,11,51844,51844,11,51900,51900,11,51956,51956,11,52012,52012,11,52068,52068,11,52124,52124,11,52180,52180,11,52236,52236,11,52292,52292,11,52348,52348,11,52404,52404,11,52460,52460,11,52516,52516,11,52572,52572,11,52628,52628,11,52684,52684,11,52740,52740,11,52796,52796,11,52852,52852,11,52908,52908,11,52964,52964,11,53020,53020,11,53076,53076,11,53132,53132,11,53188,53188,11,53244,53244,11,53300,53300,11,53356,53356,11,53412,53412,11,53468,53468,11,53524,53524,11,53580,53580,11,53636,53636,11,53692,53692,11,53748,53748,11,53804,53804,11,53860,53860,11,53916,53916,11,53972,53972,11,54028,54028,11,54084,54084,11,54140,54140,11,54196,54196,11,54252,54252,11,54308,54308,11,54364,54364,11,54420,54420,11,54476,54476,11,54532,54532,11,54588,54588,11,54644,54644,11,54700,54700,11,54756,54756,11,54812,54812,11,54868,54868,11,54924,54924,11,54980,54980,11,55036,55036,11,55092,55092,11,55148,55148,11,55216,55238,9,65056,65071,5,65529,65531,4,68097,68099,5,68159,68159,5,69446,69456,5,69688,69702,5,69808,69810,7,69815,69816,7,69821,69821,1,69888,69890,5,69932,69932,7,69957,69958,7,70016,70017,5,70067,70069,7,70079,70080,7,70089,70092,5,70095,70095,5,70191,70193,5,70196,70196,5,70198,70199,5,70367,70367,5,70371,70378,5,70402,70403,7,70462,70462,5,70464,70464,5,70471,70472,7,70487,70487,5,70502,70508,5,70709,70711,7,70720,70721,7,70725,70725,7,70750,70750,5,70833,70834,7,70841,70841,7,70843,70844,7,70846,70846,7,70849,70849,7,71087,71087,5,71090,71093,5,71100,71101,5,71103,71104,5,71216,71218,7,71227,71228,7,71230,71230,7,71339,71339,5,71341,71341,5,71344,71349,5,71351,71351,5,71456,71457,7,71462,71462,7,71724,71726,7,71736,71736,7,71984,71984,5,71991,71992,7,71997,71997,7,71999,71999,1,72001,72001,1,72003,72003,5,72148,72151,5,72156,72159,7,72164,72164,7,72243,72248,5,72250,72250,1,72263,72263,5,72279,72280,7,72324,72329,1,72343,72343,7,72751,72751,7,72760,72765,5,72767,72767,5,72873,72873,7,72881,72881,7,72884,72884,7,73009,73014,5,73020,73021,5,73030,73030,1,73098,73102,7,73107,73108,7,73110,73110,7,73459,73460,5,78896,78904,4,92976,92982,5,94033,94087,7,94180,94180,5,113821,113822,5,119141,119141,5,119143,119145,5,119150,119154,5,119163,119170,5,119210,119213,5,121344,121398,5,121461,121461,5,121499,121503,5,122880,122886,5,122907,122913,5,122918,122922,5,123628,123631,5,125252,125258,5,126980,126980,14,127183,127183,14,127245,127247,14,127340,127343,14,127358,127359,14,127377,127386,14,127462,127487,6,127491,127503,14,127535,127535,14,127548,127551,14,127568,127569,14,127744,127777,14,127780,127891,14,127894,127895,14,127897,127899,14,127902,127984,14,127987,127989,14,127991,127994,14,128000,128253,14,128255,128317,14,128329,128334,14,128336,128359,14,128367,128368,14,128371,128377,14,128379,128390,14,128392,128393,14,128398,128399,14,128401,128404,14,128407,128419,14,128421,128421,14,128424,128424,14,128433,128434,14,128444,128444,14,128450,128452,14,128465,128467,14,128476,128478,14,128481,128481,14,128483,128483,14,128488,128488,14,128495,128495,14,128499,128499,14,128506,128591,14,128710,128714,14,128721,128722,14,128725,128725,14,128728,128735,14,128742,128744,14,128746,128746,14,128749,128751,14,128753,128754,14,128756,128758,14,128761,128761,14,128763,128764,14,128884,128895,14,128992,129003,14,129036,129039,14,129114,129119,14,129198,129279,14,129293,129295,14,129305,129310,14,129312,129319,14,129328,129328,14,129331,129338,14,129343,129343,14,129351,129355,14,129357,129359,14,129375,129387,14,129393,129393,14,129395,129398,14,129401,129401,14,129403,129403,14,129408,129412,14,129426,129431,14,129443,129444,14,129451,129453,14,129456,129465,14,129472,129472,14,129475,129482,14,129484,129484,14,129488,129510,14,129536,129647,14,129652,129652,14,129656,129658,14,129664,129666,14,129671,129679,14,129686,129704,14,129712,129718,14,129728,129730,14,129744,129750,14,917504,917504,4,917506,917535,4,917632,917759,4,918000,921599,4,0,9,4,11,12,4,14,31,4,169,169,14,174,174,14,1155,1159,5,1425,1469,5,1473,1474,5,1479,1479,5,1552,1562,5,1611,1631,5,1750,1756,5,1759,1764,5,1770,1773,5,1809,1809,5,1958,1968,5,2045,2045,5,2075,2083,5,2089,2093,5,2259,2273,5,2275,2306,5,2362,2362,5,2364,2364,5,2369,2376,5,2381,2381,5,2385,2391,5,2433,2433,5,2492,2492,5,2495,2496,7,2503,2504,7,2509,2509,5,2530,2531,5,2561,2562,5,2620,2620,5,2625,2626,5,2635,2637,5,2672,2673,5,2689,2690,5,2748,2748,5,2753,2757,5,2761,2761,7,2765,2765,5,2810,2815,5,2818,2819,7,2878,2878,5,2880,2880,7,2887,2888,7,2893,2893,5,2903,2903,5,2946,2946,5,3007,3007,7,3009,3010,7,3018,3020,7,3031,3031,5,3073,3075,7,3134,3136,5,3142,3144,5,3157,3158,5,3201,3201,5,3260,3260,5,3263,3263,5,3266,3266,5,3270,3270,5,3274,3275,7,3285,3286,5,3328,3329,5,3387,3388,5,3391,3392,7,3398,3400,7,3405,3405,5,3415,3415,5,3457,3457,5,3530,3530,5,3536,3537,7,3542,3542,5,3551,3551,5,3633,3633,5,3636,3642,5,3761,3761,5,3764,3772,5,3864,3865,5,3895,3895,5,3902,3903,7,3967,3967,7,3974,3975,5,3993,4028,5,4141,4144,5,4146,4151,5,4155,4156,7,4182,4183,7,4190,4192,5,4226,4226,5,4229,4230,5,4253,4253,5,4448,4519,9,4957,4959,5,5938,5940,5,6002,6003,5,6070,6070,7,6078,6085,7,6087,6088,7,6109,6109,5,6158,6158,4,6313,6313,5,6435,6438,7,6441,6443,7,6450,6450,5,6457,6459,5,6681,6682,7,6741,6741,7,6743,6743,7,6752,6752,5,6757,6764,5,6771,6780,5,6832,6845,5,6847,6848,5,6916,6916,7,6965,6965,5,6971,6971,7,6973,6977,7,6979,6980,7,7040,7041,5,7073,7073,7,7078,7079,7,7082,7082,7,7142,7142,5,7144,7145,5,7149,7149,5,7151,7153,5,7204,7211,7,7220,7221,7,7376,7378,5,7393,7393,7,7405,7405,5,7415,7415,7,7616,7673,5,8203,8203,4,8205,8205,13,8232,8232,4,8234,8238,4,8265,8265,14,8293,8293,4,8400,8412,5,8417,8417,5,8421,8432,5,8505,8505,14,8617,8618,14,9000,9000,14,9167,9167,14,9208,9210,14,9642,9643,14,9664,9664,14,9728,9732,14,9735,9741,14,9743,9744,14,9746,9746,14,9750,9751,14,9753,9756,14,9758,9759,14,9761,9761,14,9764,9765,14,9767,9769,14,9771,9773,14,9776,9783,14,9787,9791,14,9793,9793,14,9795,9799,14,9812,9822,14,9824,9824,14,9827,9827,14,9829,9830,14,9832,9832,14,9851,9851,14,9854,9854,14,9856,9861,14,9874,9876,14,9878,9879,14,9881,9881,14,9883,9884,14,9888,9889,14,9895,9895,14,9898,9899,14,9904,9905,14,9917,9918,14,9924,9925,14,9928,9928,14,9934,9935,14,9937,9937,14,9939,9940,14,9961,9962,14,9968,9973,14,9975,9978,14,9981,9981,14,9986,9986,14,9989,9989,14,9998,9998,14,10000,10001,14,10004,10004,14,10013,10013,14,10024,10024,14,10052,10052,14,10060,10060,14,10067,10069,14,10083,10084,14,10133,10135,14,10160,10160,14,10548,10549,14,11035,11036,14,11093,11093,14,11647,11647,5,12330,12333,5,12336,12336,14,12441,12442,5,12953,12953,14,42608,42610,5,42654,42655,5,43010,43010,5,43019,43019,5,43045,43046,5,43052,43052,5,43188,43203,7,43232,43249,5,43302,43309,5,43346,43347,7,43392,43394,5,43443,43443,5,43446,43449,5,43452,43453,5,43493,43493,5,43567,43568,7,43571,43572,7,43587,43587,5,43597,43597,7,43696,43696,5,43703,43704,5,43713,43713,5,43756,43757,5,43765,43765,7,44003,44004,7,44006,44007,7,44009,44010,7,44013,44013,5,44033,44059,12,44061,44087,12,44089,44115,12,44117,44143,12,44145,44171,12,44173,44199,12,44201,44227,12,44229,44255,12,44257,44283,12,44285,44311,12,44313,44339,12,44341,44367,12,44369,44395,12,44397,44423,12,44425,44451,12,44453,44479,12,44481,44507,12,44509,44535,12,44537,44563,12,44565,44591,12,44593,44619,12,44621,44647,12,44649,44675,12,44677,44703,12,44705,44731,12,44733,44759,12,44761,44787,12,44789,44815,12,44817,44843,12,44845,44871,12,44873,44899,12,44901,44927,12,44929,44955,12,44957,44983,12,44985,45011,12,45013,45039,12,45041,45067,12,45069,45095,12,45097,45123,12,45125,45151,12,45153,45179,12,45181,45207,12,45209,45235,12,45237,45263,12,45265,45291,12,45293,45319,12,45321,45347,12,45349,45375,12,45377,45403,12,45405,45431,12,45433,45459,12,45461,45487,12,45489,45515,12,45517,45543,12,45545,45571,12,45573,45599,12,45601,45627,12,45629,45655,12,45657,45683,12,45685,45711,12,45713,45739,12,45741,45767,12,45769,45795,12,45797,45823,12,45825,45851,12,45853,45879,12,45881,45907,12,45909,45935,12,45937,45963,12,45965,45991,12,45993,46019,12,46021,46047,12,46049,46075,12,46077,46103,12,46105,46131,12,46133,46159,12,46161,46187,12,46189,46215,12,46217,46243,12,46245,46271,12,46273,46299,12,46301,46327,12,46329,46355,12,46357,46383,12,46385,46411,12,46413,46439,12,46441,46467,12,46469,46495,12,46497,46523,12,46525,46551,12,46553,46579,12,46581,46607,12,46609,46635,12,46637,46663,12,46665,46691,12,46693,46719,12,46721,46747,12,46749,46775,12,46777,46803,12,46805,46831,12,46833,46859,12,46861,46887,12,46889,46915,12,46917,46943,12,46945,46971,12,46973,46999,12,47001,47027,12,47029,47055,12,47057,47083,12,47085,47111,12,47113,47139,12,47141,47167,12,47169,47195,12,47197,47223,12,47225,47251,12,47253,47279,12,47281,47307,12,47309,47335,12,47337,47363,12,47365,47391,12,47393,47419,12,47421,47447,12,47449,47475,12,47477,47503,12,47505,47531,12,47533,47559,12,47561,47587,12,47589,47615,12,47617,47643,12,47645,47671,12,47673,47699,12,47701,47727,12,47729,47755,12,47757,47783,12,47785,47811,12,47813,47839,12,47841,47867,12,47869,47895,12,47897,47923,12,47925,47951,12,47953,47979,12,47981,48007,12,48009,48035,12,48037,48063,12,48065,48091,12,48093,48119,12,48121,48147,12,48149,48175,12,48177,48203,12,48205,48231,12,48233,48259,12,48261,48287,12,48289,48315,12,48317,48343,12,48345,48371,12,48373,48399,12,48401,48427,12,48429,48455,12,48457,48483,12,48485,48511,12,48513,48539,12,48541,48567,12,48569,48595,12,48597,48623,12,48625,48651,12,48653,48679,12,48681,48707,12,48709,48735,12,48737,48763,12,48765,48791,12,48793,48819,12,48821,48847,12,48849,48875,12,48877,48903,12,48905,48931,12,48933,48959,12,48961,48987,12,48989,49015,12,49017,49043,12,49045,49071,12,49073,49099,12,49101,49127,12,49129,49155,12,49157,49183,12,49185,49211,12,49213,49239,12,49241,49267,12,49269,49295,12,49297,49323,12,49325,49351,12,49353,49379,12,49381,49407,12,49409,49435,12,49437,49463,12,49465,49491,12,49493,49519,12,49521,49547,12,49549,49575,12,49577,49603,12,49605,49631,12,49633,49659,12,49661,49687,12,49689,49715,12,49717,49743,12,49745,49771,12,49773,49799,12,49801,49827,12,49829,49855,12,49857,49883,12,49885,49911,12,49913,49939,12,49941,49967,12,49969,49995,12,49997,50023,12,50025,50051,12,50053,50079,12,50081,50107,12,50109,50135,12,50137,50163,12,50165,50191,12,50193,50219,12,50221,50247,12,50249,50275,12,50277,50303,12,50305,50331,12,50333,50359,12,50361,50387,12,50389,50415,12,50417,50443,12,50445,50471,12,50473,50499,12,50501,50527,12,50529,50555,12,50557,50583,12,50585,50611,12,50613,50639,12,50641,50667,12,50669,50695,12,50697,50723,12,50725,50751,12,50753,50779,12,50781,50807,12,50809,50835,12,50837,50863,12,50865,50891,12,50893,50919,12,50921,50947,12,50949,50975,12,50977,51003,12,51005,51031,12,51033,51059,12,51061,51087,12,51089,51115,12,51117,51143,12,51145,51171,12,51173,51199,12,51201,51227,12,51229,51255,12,51257,51283,12,51285,51311,12,51313,51339,12,51341,51367,12,51369,51395,12,51397,51423,12,51425,51451,12,51453,51479,12,51481,51507,12,51509,51535,12,51537,51563,12,51565,51591,12,51593,51619,12,51621,51647,12,51649,51675,12,51677,51703,12,51705,51731,12,51733,51759,12,51761,51787,12,51789,51815,12,51817,51843,12,51845,51871,12,51873,51899,12,51901,51927,12,51929,51955,12,51957,51983,12,51985,52011,12,52013,52039,12,52041,52067,12,52069,52095,12,52097,52123,12,52125,52151,12,52153,52179,12,52181,52207,12,52209,52235,12,52237,52263,12,52265,52291,12,52293,52319,12,52321,52347,12,52349,52375,12,52377,52403,12,52405,52431,12,52433,52459,12,52461,52487,12,52489,52515,12,52517,52543,12,52545,52571,12,52573,52599,12,52601,52627,12,52629,52655,12,52657,52683,12,52685,52711,12,52713,52739,12,52741,52767,12,52769,52795,12,52797,52823,12,52825,52851,12,52853,52879,12,52881,52907,12,52909,52935,12,52937,52963,12,52965,52991,12,52993,53019,12,53021,53047,12,53049,53075,12,53077,53103,12,53105,53131,12,53133,53159,12,53161,53187,12,53189,53215,12,53217,53243,12,53245,53271,12,53273,53299,12,53301,53327,12,53329,53355,12,53357,53383,12,53385,53411,12,53413,53439,12,53441,53467,12,53469,53495,12,53497,53523,12,53525,53551,12,53553,53579,12,53581,53607,12,53609,53635,12,53637,53663,12,53665,53691,12,53693,53719,12,53721,53747,12,53749,53775,12,53777,53803,12,53805,53831,12,53833,53859,12,53861,53887,12,53889,53915,12,53917,53943,12,53945,53971,12,53973,53999,12,54001,54027,12,54029,54055,12,54057,54083,12,54085,54111,12,54113,54139,12,54141,54167,12,54169,54195,12,54197,54223,12,54225,54251,12,54253,54279,12,54281,54307,12,54309,54335,12,54337,54363,12,54365,54391,12,54393,54419,12,54421,54447,12,54449,54475,12,54477,54503,12,54505,54531,12,54533,54559,12,54561,54587,12,54589,54615,12,54617,54643,12,54645,54671,12,54673,54699,12,54701,54727,12,54729,54755,12,54757,54783,12,54785,54811,12,54813,54839,12,54841,54867,12,54869,54895,12,54897,54923,12,54925,54951,12,54953,54979,12,54981,55007,12,55009,55035,12,55037,55063,12,55065,55091,12,55093,55119,12,55121,55147,12,55149,55175,12,55177,55203,12,55243,55291,10,65024,65039,5,65279,65279,4,65520,65528,4,66045,66045,5,66422,66426,5,68101,68102,5,68152,68154,5,68325,68326,5,69291,69292,5,69632,69632,7,69634,69634,7,69759,69761,5]');
    }
});
//#endregion

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[9/*vs/base/common/types*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var _typeof = {
        number: 'number',
        string: 'string',
        undefined: 'undefined',
        object: 'object',
        function: 'function'
    };
    /**
     * @returns whether the provided parameter is a JavaScript Array or not.
     */
    function isArray(array) {
        if (Array.isArray) {
            return Array.isArray(array);
        }
        if (array && typeof (array.length) === _typeof.number && array.constructor === Array) {
            return true;
        }
        return false;
    }
    exports.isArray = isArray;
    /**
     * @returns whether the provided parameter is a JavaScript String or not.
     */
    function isString(str) {
        if (typeof (str) === _typeof.string || str instanceof String) {
            return true;
        }
        return false;
    }
    exports.isString = isString;
    /**
     * @returns whether the provided parameter is a JavaScript Array and each element in the array is a string.
     */
    function isStringArray(value) {
        return isArray(value) && value.every(function (elem) { return isString(elem); });
    }
    exports.isStringArray = isStringArray;
    /**
     *
     * @returns whether the provided parameter is of type `object` but **not**
     *	`null`, an `array`, a `regexp`, nor a `date`.
     */
    function isObject(obj) {
        // The method can't do a type cast since there are type (like strings) which
        // are subclasses of any put not positvely matched by the function. Hence type
        // narrowing results in wrong results.
        return typeof obj === _typeof.object
            && obj !== null
            && !Array.isArray(obj)
            && !(obj instanceof RegExp)
            && !(obj instanceof Date);
    }
    exports.isObject = isObject;
    /**
     * In **contrast** to just checking `typeof` this will return `false` for `NaN`.
     * @returns whether the provided parameter is a JavaScript Number or not.
     */
    function isNumber(obj) {
        if ((typeof (obj) === _typeof.number || obj instanceof Number) && !isNaN(obj)) {
            return true;
        }
        return false;
    }
    exports.isNumber = isNumber;
    /**
     * @returns whether the provided parameter is a JavaScript Boolean or not.
     */
    function isBoolean(obj) {
        return obj === true || obj === false;
    }
    exports.isBoolean = isBoolean;
    /**
     * @returns whether the provided parameter is undefined.
     */
    function isUndefined(obj) {
        return typeof (obj) === _typeof.undefined;
    }
    exports.isUndefined = isUndefined;
    /**
     * @returns whether the provided parameter is undefined or null.
     */
    function isUndefinedOrNull(obj) {
        return isUndefined(obj) || obj === null;
    }
    exports.isUndefinedOrNull = isUndefinedOrNull;
    function assertType(condition, type) {
        if (!condition) {
            throw new Error(type ? "Unexpected type, expected '" + type + "'" : 'Unexpected type');
        }
    }
    exports.assertType = assertType;
    /**
     * Asserts that the argument passed in is neither undefined nor null.
     */
    function assertIsDefined(arg) {
        if (isUndefinedOrNull(arg)) {
            throw new Error('Assertion Failed: argument is undefined or null');
        }
        return arg;
    }
    exports.assertIsDefined = assertIsDefined;
    function assertAllDefined() {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        var result = [];
        for (var i = 0; i < args.length; i++) {
            var arg = args[i];
            if (isUndefinedOrNull(arg)) {
                throw new Error("Assertion Failed: argument at index " + i + " is undefined or null");
            }
            result.push(arg);
        }
        return result;
    }
    exports.assertAllDefined = assertAllDefined;
    var hasOwnProperty = Object.prototype.hasOwnProperty;
    /**
     * @returns whether the provided parameter is an empty JavaScript Object or not.
     */
    function isEmptyObject(obj) {
        if (!isObject(obj)) {
            return false;
        }
        for (var key in obj) {
            if (hasOwnProperty.call(obj, key)) {
                return false;
            }
        }
        return true;
    }
    exports.isEmptyObject = isEmptyObject;
    /**
     * @returns whether the provided parameter is a JavaScript Function or not.
     */
    function isFunction(obj) {
        return typeof obj === _typeof.function;
    }
    exports.isFunction = isFunction;
    /**
     * @returns whether the provided parameters is are JavaScript Function or not.
     */
    function areFunctions() {
        var objects = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            objects[_i] = arguments[_i];
        }
        return objects.length > 0 && objects.every(isFunction);
    }
    exports.areFunctions = areFunctions;
    function validateConstraints(args, constraints) {
        var len = Math.min(args.length, constraints.length);
        for (var i = 0; i < len; i++) {
            validateConstraint(args[i], constraints[i]);
        }
    }
    exports.validateConstraints = validateConstraints;
    function validateConstraint(arg, constraint) {
        if (isString(constraint)) {
            if (typeof arg !== constraint) {
                throw new Error("argument does not match constraint: typeof " + constraint);
            }
        }
        else if (isFunction(constraint)) {
            try {
                if (arg instanceof constraint) {
                    return;
                }
            }
            catch (_a) {
                // ignore
            }
            if (!isUndefinedOrNull(arg) && arg.constructor === constraint) {
                return;
            }
            if (constraint.length === 1 && constraint.call(undefined, arg) === true) {
                return;
            }
            throw new Error("argument does not match one of these constraints: arg instanceof constraint, arg.constructor === constraint, nor constraint(arg) === true");
        }
    }
    exports.validateConstraint = validateConstraint;
    function getAllPropertyNames(obj) {
        var res = [];
        var proto = Object.getPrototypeOf(obj);
        while (Object.prototype !== proto) {
            res = res.concat(Object.getOwnPropertyNames(proto));
            proto = Object.getPrototypeOf(proto);
        }
        return res;
    }
    exports.getAllPropertyNames = getAllPropertyNames;
    function getAllMethodNames(obj) {
        var methods = [];
        for (var _i = 0, _a = getAllPropertyNames(obj); _i < _a.length; _i++) {
            var prop = _a[_i];
            if (typeof obj[prop] === 'function') {
                methods.push(prop);
            }
        }
        return methods;
    }
    exports.getAllMethodNames = getAllMethodNames;
    function createProxyObject(methodNames, invoke) {
        var createProxyMethod = function (method) {
            return function () {
                var args = Array.prototype.slice.call(arguments, 0);
                return invoke(method, args);
            };
        };
        var result = {};
        for (var _i = 0, methodNames_1 = methodNames; _i < methodNames_1.length; _i++) {
            var methodName = methodNames_1[_i];
            result[methodName] = createProxyMethod(methodName);
        }
        return result;
    }
    exports.createProxyObject = createProxyObject;
    /**
     * Converts null to undefined, passes all other values through.
     */
    function withNullAsUndefined(x) {
        return x === null ? undefined : x;
    }
    exports.withNullAsUndefined = withNullAsUndefined;
    /**
     * Converts undefined to null, passes all other values through.
     */
    function withUndefinedAsNull(x) {
        return typeof x === 'undefined' ? null : x;
    }
    exports.withUndefinedAsNull = withUndefinedAsNull;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[7/*vs/base/common/uint*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    function toUint8(v) {
        if (v < 0) {
            return 0;
        }
        if (v > 255 /* MAX_UINT_8 */) {
            return 255 /* MAX_UINT_8 */;
        }
        return v | 0;
    }
    exports.toUint8 = toUint8;
    function toUint32(v) {
        if (v < 0) {
            return 0;
        }
        if (v > 4294967295 /* MAX_UINT_32 */) {
            return 4294967295 /* MAX_UINT_32 */;
        }
        return v | 0;
    }
    exports.toUint32 = toUint32;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/













define(__m[11/*vs/base/common/uri*/], __M([1/*require*/,0/*exports*/,5/*vs/base/common/platform*/]), function (require, exports, platform_1) {
    "use strict";
    var _a;
    Object.defineProperty(exports, "__esModule", { value: true });
    var _schemePattern = /^\w[\w\d+.-]*$/;
    var _singleSlashStart = /^\//;
    var _doubleSlashStart = /^\/\//;
    function _validateUri(ret, _strict) {
        // scheme, must be set
        if (!ret.scheme && _strict) {
            throw new Error("[UriError]: Scheme is missing: {scheme: \"\", authority: \"" + ret.authority + "\", path: \"" + ret.path + "\", query: \"" + ret.query + "\", fragment: \"" + ret.fragment + "\"}");
        }
        // scheme, https://tools.ietf.org/html/rfc3986#section-3.1
        // ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
        if (ret.scheme && !_schemePattern.test(ret.scheme)) {
            throw new Error('[UriError]: Scheme contains illegal characters.');
        }
        // path, http://tools.ietf.org/html/rfc3986#section-3.3
        // If a URI contains an authority component, then the path component
        // must either be empty or begin with a slash ("/") character.  If a URI
        // does not contain an authority component, then the path cannot begin
        // with two slash characters ("//").
        if (ret.path) {
            if (ret.authority) {
                if (!_singleSlashStart.test(ret.path)) {
                    throw new Error('[UriError]: If a URI contains an authority component, then the path component must either be empty or begin with a slash ("/") character');
                }
            }
            else {
                if (_doubleSlashStart.test(ret.path)) {
                    throw new Error('[UriError]: If a URI does not contain an authority component, then the path cannot begin with two slash characters ("//")');
                }
            }
        }
    }
    // for a while we allowed uris *without* schemes and this is the migration
    // for them, e.g. an uri without scheme and without strict-mode warns and falls
    // back to the file-scheme. that should cause the least carnage and still be a
    // clear warning
    function _schemeFix(scheme, _strict) {
        if (!scheme && !_strict) {
            return 'file';
        }
        return scheme;
    }
    // implements a bit of https://tools.ietf.org/html/rfc3986#section-5
    function _referenceResolution(scheme, path) {
        // the slash-character is our 'default base' as we don't
        // support constructing URIs relative to other URIs. This
        // also means that we alter and potentially break paths.
        // see https://tools.ietf.org/html/rfc3986#section-5.1.4
        switch (scheme) {
            case 'https':
            case 'http':
            case 'file':
                if (!path) {
                    path = _slash;
                }
                else if (path[0] !== _slash) {
                    path = _slash + path;
                }
                break;
        }
        return path;
    }
    var _empty = '';
    var _slash = '/';
    var _regexp = /^(([^:/?#]+?):)?(\/\/([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?/;
    /**
     * Uniform Resource Identifier (URI) http://tools.ietf.org/html/rfc3986.
     * This class is a simple parser which creates the basic component parts
     * (http://tools.ietf.org/html/rfc3986#section-3) with minimal validation
     * and encoding.
     *
     *       foo://example.com:8042/over/there?name=ferret#nose
     *       \_/   \______________/\_________/ \_________/ \__/
     *        |           |            |            |        |
     *     scheme     authority       path        query   fragment
     *        |   _____________________|__
     *       / \ /                        \
     *       urn:example:animal:ferret:nose
     */
    var URI = /** @class */ (function () {
        /**
         * @internal
         */
        function URI(schemeOrData, authority, path, query, fragment, _strict) {
            if (_strict === void 0) { _strict = false; }
            if (typeof schemeOrData === 'object') {
                this.scheme = schemeOrData.scheme || _empty;
                this.authority = schemeOrData.authority || _empty;
                this.path = schemeOrData.path || _empty;
                this.query = schemeOrData.query || _empty;
                this.fragment = schemeOrData.fragment || _empty;
                // no validation because it's this URI
                // that creates uri components.
                // _validateUri(this);
            }
            else {
                this.scheme = _schemeFix(schemeOrData, _strict);
                this.authority = authority || _empty;
                this.path = _referenceResolution(this.scheme, path || _empty);
                this.query = query || _empty;
                this.fragment = fragment || _empty;
                _validateUri(this, _strict);
            }
        }
        URI.isUri = function (thing) {
            if (thing instanceof URI) {
                return true;
            }
            if (!thing) {
                return false;
            }
            return typeof thing.authority === 'string'
                && typeof thing.fragment === 'string'
                && typeof thing.path === 'string'
                && typeof thing.query === 'string'
                && typeof thing.scheme === 'string'
                && typeof thing.fsPath === 'function'
                && typeof thing.with === 'function'
                && typeof thing.toString === 'function';
        };
        Object.defineProperty(URI.prototype, "fsPath", {
            // ---- filesystem path -----------------------
            /**
             * Returns a string representing the corresponding file system path of this URI.
             * Will handle UNC paths, normalizes windows drive letters to lower-case, and uses the
             * platform specific path separator.
             *
             * * Will *not* validate the path for invalid characters and semantics.
             * * Will *not* look at the scheme of this URI.
             * * The result shall *not* be used for display purposes but for accessing a file on disk.
             *
             *
             * The *difference* to `URI#path` is the use of the platform specific separator and the handling
             * of UNC paths. See the below sample of a file-uri with an authority (UNC path).
             *
             * ```ts
                const u = URI.parse('file://server/c$/folder/file.txt')
                u.authority === 'server'
                u.path === '/shares/c$/file.txt'
                u.fsPath === '\\server\c$\folder\file.txt'
            ```
             *
             * Using `URI#path` to read a file (using fs-apis) would not be enough because parts of the path,
             * namely the server name, would be missing. Therefore `URI#fsPath` exists - it's sugar to ease working
             * with URIs that represent files on disk (`file` scheme).
             */
            get: function () {
                // if (this.scheme !== 'file') {
                // 	console.warn(`[UriError] calling fsPath with scheme ${this.scheme}`);
                // }
                return _makeFsPath(this);
            },
            enumerable: true,
            configurable: true
        });
        // ---- modify to new -------------------------
        URI.prototype.with = function (change) {
            if (!change) {
                return this;
            }
            var scheme = change.scheme, authority = change.authority, path = change.path, query = change.query, fragment = change.fragment;
            if (scheme === undefined) {
                scheme = this.scheme;
            }
            else if (scheme === null) {
                scheme = _empty;
            }
            if (authority === undefined) {
                authority = this.authority;
            }
            else if (authority === null) {
                authority = _empty;
            }
            if (path === undefined) {
                path = this.path;
            }
            else if (path === null) {
                path = _empty;
            }
            if (query === undefined) {
                query = this.query;
            }
            else if (query === null) {
                query = _empty;
            }
            if (fragment === undefined) {
                fragment = this.fragment;
            }
            else if (fragment === null) {
                fragment = _empty;
            }
            if (scheme === this.scheme
                && authority === this.authority
                && path === this.path
                && query === this.query
                && fragment === this.fragment) {
                return this;
            }
            return new _URI(scheme, authority, path, query, fragment);
        };
        // ---- parse & validate ------------------------
        /**
         * Creates a new URI from a string, e.g. `http://www.msft.com/some/path`,
         * `file:///usr/home`, or `scheme:with/path`.
         *
         * @param value A string which represents an URI (see `URI#toString`).
         */
        URI.parse = function (value, _strict) {
            if (_strict === void 0) { _strict = false; }
            var match = _regexp.exec(value);
            if (!match) {
                return new _URI(_empty, _empty, _empty, _empty, _empty);
            }
            return new _URI(match[2] || _empty, percentDecode(match[4] || _empty), percentDecode(match[5] || _empty), percentDecode(match[7] || _empty), percentDecode(match[9] || _empty), _strict);
        };
        /**
         * Creates a new URI from a file system path, e.g. `c:\my\files`,
         * `/usr/home`, or `\\server\share\some\path`.
         *
         * The *difference* between `URI#parse` and `URI#file` is that the latter treats the argument
         * as path, not as stringified-uri. E.g. `URI.file(path)` is **not the same as**
         * `URI.parse('file://' + path)` because the path might contain characters that are
         * interpreted (# and ?). See the following sample:
         * ```ts
        const good = URI.file('/coding/c#/project1');
        good.scheme === 'file';
        good.path === '/coding/c#/project1';
        good.fragment === '';
        const bad = URI.parse('file://' + '/coding/c#/project1');
        bad.scheme === 'file';
        bad.path === '/coding/c'; // path is now broken
        bad.fragment === '/project1';
        ```
         *
         * @param path A file system path (see `URI#fsPath`)
         */
        URI.file = function (path) {
            var authority = _empty;
            // normalize to fwd-slashes on windows,
            // on other systems bwd-slashes are valid
            // filename character, eg /f\oo/ba\r.txt
            if (platform_1.isWindows) {
                path = path.replace(/\\/g, _slash);
            }
            // check for authority as used in UNC shares
            // or use the path as given
            if (path[0] === _slash && path[1] === _slash) {
                var idx = path.indexOf(_slash, 2);
                if (idx === -1) {
                    authority = path.substring(2);
                    path = _slash;
                }
                else {
                    authority = path.substring(2, idx);
                    path = path.substring(idx) || _slash;
                }
            }
            return new _URI('file', authority, path, _empty, _empty);
        };
        URI.from = function (components) {
            return new _URI(components.scheme, components.authority, components.path, components.query, components.fragment);
        };
        // ---- printing/externalize ---------------------------
        /**
         * Creates a string representation for this URI. It's guaranteed that calling
         * `URI.parse` with the result of this function creates an URI which is equal
         * to this URI.
         *
         * * The result shall *not* be used for display purposes but for externalization or transport.
         * * The result will be encoded using the percentage encoding and encoding happens mostly
         * ignore the scheme-specific encoding rules.
         *
         * @param skipEncoding Do not encode the result, default is `false`
         */
        URI.prototype.toString = function (skipEncoding) {
            if (skipEncoding === void 0) { skipEncoding = false; }
            return _asFormatted(this, skipEncoding);
        };
        URI.prototype.toJSON = function () {
            return this;
        };
        URI.revive = function (data) {
            if (!data) {
                return data;
            }
            else if (data instanceof URI) {
                return data;
            }
            else {
                var result = new _URI(data);
                result._formatted = data.external;
                result._fsPath = data._sep === _pathSepMarker ? data.fsPath : null;
                return result;
            }
        };
        return URI;
    }());
    exports.URI = URI;
    var _pathSepMarker = platform_1.isWindows ? 1 : undefined;
    // eslint-disable-next-line @typescript-eslint/class-name-casing
    var _URI = /** @class */ (function (_super) {
        __extends(_URI, _super);
        function _URI() {
            var _this = _super !== null && _super.apply(this, arguments) || this;
            _this._formatted = null;
            _this._fsPath = null;
            return _this;
        }
        Object.defineProperty(_URI.prototype, "fsPath", {
            get: function () {
                if (!this._fsPath) {
                    this._fsPath = _makeFsPath(this);
                }
                return this._fsPath;
            },
            enumerable: true,
            configurable: true
        });
        _URI.prototype.toString = function (skipEncoding) {
            if (skipEncoding === void 0) { skipEncoding = false; }
            if (!skipEncoding) {
                if (!this._formatted) {
                    this._formatted = _asFormatted(this, false);
                }
                return this._formatted;
            }
            else {
                // we don't cache that
                return _asFormatted(this, true);
            }
        };
        _URI.prototype.toJSON = function () {
            var res = {
                $mid: 1
            };
            // cached state
            if (this._fsPath) {
                res.fsPath = this._fsPath;
                res._sep = _pathSepMarker;
            }
            if (this._formatted) {
                res.external = this._formatted;
            }
            // uri components
            if (this.path) {
                res.path = this.path;
            }
            if (this.scheme) {
                res.scheme = this.scheme;
            }
            if (this.authority) {
                res.authority = this.authority;
            }
            if (this.query) {
                res.query = this.query;
            }
            if (this.fragment) {
                res.fragment = this.fragment;
            }
            return res;
        };
        return _URI;
    }(URI));
    // reserved characters: https://tools.ietf.org/html/rfc3986#section-2.2
    var encodeTable = (_a = {},
        _a[58 /* Colon */] = '%3A',
        _a[47 /* Slash */] = '%2F',
        _a[63 /* QuestionMark */] = '%3F',
        _a[35 /* Hash */] = '%23',
        _a[91 /* OpenSquareBracket */] = '%5B',
        _a[93 /* CloseSquareBracket */] = '%5D',
        _a[64 /* AtSign */] = '%40',
        _a[33 /* ExclamationMark */] = '%21',
        _a[36 /* DollarSign */] = '%24',
        _a[38 /* Ampersand */] = '%26',
        _a[39 /* SingleQuote */] = '%27',
        _a[40 /* OpenParen */] = '%28',
        _a[41 /* CloseParen */] = '%29',
        _a[42 /* Asterisk */] = '%2A',
        _a[43 /* Plus */] = '%2B',
        _a[44 /* Comma */] = '%2C',
        _a[59 /* Semicolon */] = '%3B',
        _a[61 /* Equals */] = '%3D',
        _a[32 /* Space */] = '%20',
        _a);
    function encodeURIComponentFast(uriComponent, allowSlash) {
        var res = undefined;
        var nativeEncodePos = -1;
        for (var pos = 0; pos < uriComponent.length; pos++) {
            var code = uriComponent.charCodeAt(pos);
            // unreserved characters: https://tools.ietf.org/html/rfc3986#section-2.3
            if ((code >= 97 /* a */ && code <= 122 /* z */)
                || (code >= 65 /* A */ && code <= 90 /* Z */)
                || (code >= 48 /* Digit0 */ && code <= 57 /* Digit9 */)
                || code === 45 /* Dash */
                || code === 46 /* Period */
                || code === 95 /* Underline */
                || code === 126 /* Tilde */
                || (allowSlash && code === 47 /* Slash */)) {
                // check if we are delaying native encode
                if (nativeEncodePos !== -1) {
                    res += encodeURIComponent(uriComponent.substring(nativeEncodePos, pos));
                    nativeEncodePos = -1;
                }
                // check if we write into a new string (by default we try to return the param)
                if (res !== undefined) {
                    res += uriComponent.charAt(pos);
                }
            }
            else {
                // encoding needed, we need to allocate a new string
                if (res === undefined) {
                    res = uriComponent.substr(0, pos);
                }
                // check with default table first
                var escaped = encodeTable[code];
                if (escaped !== undefined) {
                    // check if we are delaying native encode
                    if (nativeEncodePos !== -1) {
                        res += encodeURIComponent(uriComponent.substring(nativeEncodePos, pos));
                        nativeEncodePos = -1;
                    }
                    // append escaped variant to result
                    res += escaped;
                }
                else if (nativeEncodePos === -1) {
                    // use native encode only when needed
                    nativeEncodePos = pos;
                }
            }
        }
        if (nativeEncodePos !== -1) {
            res += encodeURIComponent(uriComponent.substring(nativeEncodePos));
        }
        return res !== undefined ? res : uriComponent;
    }
    function encodeURIComponentMinimal(path) {
        var res = undefined;
        for (var pos = 0; pos < path.length; pos++) {
            var code = path.charCodeAt(pos);
            if (code === 35 /* Hash */ || code === 63 /* QuestionMark */) {
                if (res === undefined) {
                    res = path.substr(0, pos);
                }
                res += encodeTable[code];
            }
            else {
                if (res !== undefined) {
                    res += path[pos];
                }
            }
        }
        return res !== undefined ? res : path;
    }
    /**
     * Compute `fsPath` for the given uri
     */
    function _makeFsPath(uri) {
        var value;
        if (uri.authority && uri.path.length > 1 && uri.scheme === 'file') {
            // unc path: file://shares/c$/far/boo
            value = "//" + uri.authority + uri.path;
        }
        else if (uri.path.charCodeAt(0) === 47 /* Slash */
            && (uri.path.charCodeAt(1) >= 65 /* A */ && uri.path.charCodeAt(1) <= 90 /* Z */ || uri.path.charCodeAt(1) >= 97 /* a */ && uri.path.charCodeAt(1) <= 122 /* z */)
            && uri.path.charCodeAt(2) === 58 /* Colon */) {
            // windows drive letter: file:///c:/far/boo
            value = uri.path[1].toLowerCase() + uri.path.substr(2);
        }
        else {
            // other path
            value = uri.path;
        }
        if (platform_1.isWindows) {
            value = value.replace(/\//g, '\\');
        }
        return value;
    }
    /**
     * Create the external version of a uri
     */
    function _asFormatted(uri, skipEncoding) {
        var encoder = !skipEncoding
            ? encodeURIComponentFast
            : encodeURIComponentMinimal;
        var res = '';
        var scheme = uri.scheme, authority = uri.authority, path = uri.path, query = uri.query, fragment = uri.fragment;
        if (scheme) {
            res += scheme;
            res += ':';
        }
        if (authority || scheme === 'file') {
            res += _slash;
            res += _slash;
        }
        if (authority) {
            var idx = authority.indexOf('@');
            if (idx !== -1) {
                // <user>@<auth>
                var userinfo = authority.substr(0, idx);
                authority = authority.substr(idx + 1);
                idx = userinfo.indexOf(':');
                if (idx === -1) {
                    res += encoder(userinfo, false);
                }
                else {
                    // <user>:<pass>@<auth>
                    res += encoder(userinfo.substr(0, idx), false);
                    res += ':';
                    res += encoder(userinfo.substr(idx + 1), false);
                }
                res += '@';
            }
            authority = authority.toLowerCase();
            idx = authority.indexOf(':');
            if (idx === -1) {
                res += encoder(authority, false);
            }
            else {
                // <auth>:<port>
                res += encoder(authority.substr(0, idx), false);
                res += authority.substr(idx);
            }
        }
        if (path) {
            // lower-case windows drive letters in /C:/fff or C:/fff
            if (path.length >= 3 && path.charCodeAt(0) === 47 /* Slash */ && path.charCodeAt(2) === 58 /* Colon */) {
                var code = path.charCodeAt(1);
                if (code >= 65 /* A */ && code <= 90 /* Z */) {
                    path = "/" + String.fromCharCode(code + 32) + ":" + path.substr(3); // "/c:".length === 3
                }
            }
            else if (path.length >= 2 && path.charCodeAt(1) === 58 /* Colon */) {
                var code = path.charCodeAt(0);
                if (code >= 65 /* A */ && code <= 90 /* Z */) {
                    path = String.fromCharCode(code + 32) + ":" + path.substr(2); // "/c:".length === 3
                }
            }
            // encode the rest of the path
            res += encoder(path, true);
        }
        if (query) {
            res += '?';
            res += encoder(query, false);
        }
        if (fragment) {
            res += '#';
            res += !skipEncoding ? encodeURIComponentFast(fragment, false) : fragment;
        }
        return res;
    }
    // --- decode
    function decodeURIComponentGraceful(str) {
        try {
            return decodeURIComponent(str);
        }
        catch (_a) {
            if (str.length > 3) {
                return str.substr(0, 3) + decodeURIComponentGraceful(str.substr(3));
            }
            else {
                return str;
            }
        }
    }
    var _rEncodedAsHex = /(%[0-9A-Za-z][0-9A-Za-z])+/g;
    function percentDecode(str) {
        if (!str.match(_rEncodedAsHex)) {
            return str;
        }
        return str.replace(_rEncodedAsHex, function (match) { return decodeURIComponentGraceful(match); });
    }
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/













define(__m[33/*vs/base/common/worker/simpleWorker*/], __M([1/*require*/,0/*exports*/,3/*vs/base/common/errors*/,6/*vs/base/common/lifecycle*/,5/*vs/base/common/platform*/,9/*vs/base/common/types*/]), function (require, exports, errors_1, lifecycle_1, platform_1, types) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var INITIALIZE = '$initialize';
    var webWorkerWarningLogged = false;
    function logOnceWebWorkerWarning(err) {
        if (!platform_1.isWeb) {
            // running tests
            return;
        }
        if (!webWorkerWarningLogged) {
            webWorkerWarningLogged = true;
            console.warn('Could not create web worker(s). Falling back to loading web worker code in main thread, which might cause UI freezes. Please see https://github.com/Microsoft/monaco-editor#faq');
        }
        console.warn(err.message);
    }
    exports.logOnceWebWorkerWarning = logOnceWebWorkerWarning;
    var SimpleWorkerProtocol = /** @class */ (function () {
        function SimpleWorkerProtocol(handler) {
            this._workerId = -1;
            this._handler = handler;
            this._lastSentReq = 0;
            this._pendingReplies = Object.create(null);
        }
        SimpleWorkerProtocol.prototype.setWorkerId = function (workerId) {
            this._workerId = workerId;
        };
        SimpleWorkerProtocol.prototype.sendMessage = function (method, args) {
            var _this = this;
            var req = String(++this._lastSentReq);
            return new Promise(function (resolve, reject) {
                _this._pendingReplies[req] = {
                    resolve: resolve,
                    reject: reject
                };
                _this._send({
                    vsWorker: _this._workerId,
                    req: req,
                    method: method,
                    args: args
                });
            });
        };
        SimpleWorkerProtocol.prototype.handleMessage = function (message) {
            if (!message || !message.vsWorker) {
                return;
            }
            if (this._workerId !== -1 && message.vsWorker !== this._workerId) {
                return;
            }
            this._handleMessage(message);
        };
        SimpleWorkerProtocol.prototype._handleMessage = function (msg) {
            var _this = this;
            if (msg.seq) {
                var replyMessage = msg;
                if (!this._pendingReplies[replyMessage.seq]) {
                    console.warn('Got reply to unknown seq');
                    return;
                }
                var reply = this._pendingReplies[replyMessage.seq];
                delete this._pendingReplies[replyMessage.seq];
                if (replyMessage.err) {
                    var err = replyMessage.err;
                    if (replyMessage.err.$isError) {
                        err = new Error();
                        err.name = replyMessage.err.name;
                        err.message = replyMessage.err.message;
                        err.stack = replyMessage.err.stack;
                    }
                    reply.reject(err);
                    return;
                }
                reply.resolve(replyMessage.res);
                return;
            }
            var requestMessage = msg;
            var req = requestMessage.req;
            var result = this._handler.handleMessage(requestMessage.method, requestMessage.args);
            result.then(function (r) {
                _this._send({
                    vsWorker: _this._workerId,
                    seq: req,
                    res: r,
                    err: undefined
                });
            }, function (e) {
                if (e.detail instanceof Error) {
                    // Loading errors have a detail property that points to the actual error
                    e.detail = errors_1.transformErrorForSerialization(e.detail);
                }
                _this._send({
                    vsWorker: _this._workerId,
                    seq: req,
                    res: undefined,
                    err: errors_1.transformErrorForSerialization(e)
                });
            });
        };
        SimpleWorkerProtocol.prototype._send = function (msg) {
            var transfer = [];
            if (msg.req) {
                var m = msg;
                for (var i = 0; i < m.args.length; i++) {
                    if (m.args[i] instanceof ArrayBuffer) {
                        transfer.push(m.args[i]);
                    }
                }
            }
            else {
                var m = msg;
                if (m.res instanceof ArrayBuffer) {
                    transfer.push(m.res);
                }
            }
            this._handler.sendMessage(msg, transfer);
        };
        return SimpleWorkerProtocol;
    }());
    /**
     * Main thread side
     */
    var SimpleWorkerClient = /** @class */ (function (_super) {
        __extends(SimpleWorkerClient, _super);
        function SimpleWorkerClient(workerFactory, moduleId, host) {
            var _this = _super.call(this) || this;
            var lazyProxyReject = null;
            _this._worker = _this._register(workerFactory.create('vs/base/common/worker/simpleWorker', function (msg) {
                _this._protocol.handleMessage(msg);
            }, function (err) {
                // in Firefox, web workers fail lazily :(
                // we will reject the proxy
                if (lazyProxyReject) {
                    lazyProxyReject(err);
                }
            }));
            _this._protocol = new SimpleWorkerProtocol({
                sendMessage: function (msg, transfer) {
                    _this._worker.postMessage(msg, transfer);
                },
                handleMessage: function (method, args) {
                    if (typeof host[method] !== 'function') {
                        return Promise.reject(new Error('Missing method ' + method + ' on main thread host.'));
                    }
                    try {
                        return Promise.resolve(host[method].apply(host, args));
                    }
                    catch (e) {
                        return Promise.reject(e);
                    }
                }
            });
            _this._protocol.setWorkerId(_this._worker.getId());
            // Gather loader configuration
            var loaderConfiguration = null;
            if (typeof self.require !== 'undefined' && typeof self.require.getConfig === 'function') {
                // Get the configuration from the Monaco AMD Loader
                loaderConfiguration = self.require.getConfig();
            }
            else if (typeof self.requirejs !== 'undefined') {
                // Get the configuration from requirejs
                loaderConfiguration = self.requirejs.s.contexts._.config;
            }
            var hostMethods = types.getAllMethodNames(host);
            // Send initialize message
            _this._onModuleLoaded = _this._protocol.sendMessage(INITIALIZE, [
                _this._worker.getId(),
                JSON.parse(JSON.stringify(loaderConfiguration)),
                moduleId,
                hostMethods,
            ]);
            // Create proxy to loaded code
            var proxyMethodRequest = function (method, args) {
                return _this._request(method, args);
            };
            _this._lazyProxy = new Promise(function (resolve, reject) {
                lazyProxyReject = reject;
                _this._onModuleLoaded.then(function (availableMethods) {
                    resolve(types.createProxyObject(availableMethods, proxyMethodRequest));
                }, function (e) {
                    reject(e);
                    _this._onError('Worker failed to load ' + moduleId, e);
                });
            });
            return _this;
        }
        SimpleWorkerClient.prototype.getProxyObject = function () {
            return this._lazyProxy;
        };
        SimpleWorkerClient.prototype._request = function (method, args) {
            var _this = this;
            return new Promise(function (resolve, reject) {
                _this._onModuleLoaded.then(function () {
                    _this._protocol.sendMessage(method, args).then(resolve, reject);
                }, reject);
            });
        };
        SimpleWorkerClient.prototype._onError = function (message, error) {
            console.error(message);
            console.info(error);
        };
        return SimpleWorkerClient;
    }(lifecycle_1.Disposable));
    exports.SimpleWorkerClient = SimpleWorkerClient;
    /**
     * Worker side
     */
    var SimpleWorkerServer = /** @class */ (function () {
        function SimpleWorkerServer(postMessage, requestHandlerFactory) {
            var _this = this;
            this._requestHandlerFactory = requestHandlerFactory;
            this._requestHandler = null;
            this._protocol = new SimpleWorkerProtocol({
                sendMessage: function (msg, transfer) {
                    postMessage(msg, transfer);
                },
                handleMessage: function (method, args) { return _this._handleMessage(method, args); }
            });
        }
        SimpleWorkerServer.prototype.onmessage = function (msg) {
            this._protocol.handleMessage(msg);
        };
        SimpleWorkerServer.prototype._handleMessage = function (method, args) {
            if (method === INITIALIZE) {
                return this.initialize(args[0], args[1], args[2], args[3]);
            }
            if (!this._requestHandler || typeof this._requestHandler[method] !== 'function') {
                return Promise.reject(new Error('Missing requestHandler or method: ' + method));
            }
            try {
                return Promise.resolve(this._requestHandler[method].apply(this._requestHandler, args));
            }
            catch (e) {
                return Promise.reject(e);
            }
        };
        SimpleWorkerServer.prototype.initialize = function (workerId, loaderConfig, moduleId, hostMethods) {
            var _this = this;
            this._protocol.setWorkerId(workerId);
            var proxyMethodRequest = function (method, args) {
                return _this._protocol.sendMessage(method, args);
            };
            var hostProxy = types.createProxyObject(hostMethods, proxyMethodRequest);
            if (this._requestHandlerFactory) {
                // static request handler
                this._requestHandler = this._requestHandlerFactory(hostProxy);
                return Promise.resolve(types.getAllMethodNames(this._requestHandler));
            }
            if (loaderConfig) {
                // Remove 'baseUrl', handling it is beyond scope for now
                if (typeof loaderConfig.baseUrl !== 'undefined') {
                    delete loaderConfig['baseUrl'];
                }
                if (typeof loaderConfig.paths !== 'undefined') {
                    if (typeof loaderConfig.paths.vs !== 'undefined') {
                        delete loaderConfig.paths['vs'];
                    }
                }
                // Since this is in a web worker, enable catching errors
                loaderConfig.catchError = true;
                self.require.config(loaderConfig);
            }
            return new Promise(function (resolve, reject) {
                // Use the global require to be sure to get the global config
                self.require([moduleId], function (module) {
                    _this._requestHandler = module.create(hostProxy);
                    if (!_this._requestHandler) {
                        reject(new Error("No RequestHandler!"));
                        return;
                    }
                    resolve(types.getAllMethodNames(_this._requestHandler));
                }, reject);
            });
        };
        return SimpleWorkerServer;
    }());
    exports.SimpleWorkerServer = SimpleWorkerServer;
    /**
     * Called on the worker side
     */
    function create(postMessage) {
        return new SimpleWorkerServer(postMessage, null);
    }
    exports.create = create;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[20/*vs/editor/common/core/characterClassifier*/], __M([1/*require*/,0/*exports*/,7/*vs/base/common/uint*/]), function (require, exports, uint_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * A fast character classifier that uses a compact array for ASCII values.
     */
    var CharacterClassifier = /** @class */ (function () {
        function CharacterClassifier(_defaultValue) {
            var defaultValue = uint_1.toUint8(_defaultValue);
            this._defaultValue = defaultValue;
            this._asciiMap = CharacterClassifier._createAsciiMap(defaultValue);
            this._map = new Map();
        }
        CharacterClassifier._createAsciiMap = function (defaultValue) {
            var asciiMap = new Uint8Array(256);
            for (var i = 0; i < 256; i++) {
                asciiMap[i] = defaultValue;
            }
            return asciiMap;
        };
        CharacterClassifier.prototype.set = function (charCode, _value) {
            var value = uint_1.toUint8(_value);
            if (charCode >= 0 && charCode < 256) {
                this._asciiMap[charCode] = value;
            }
            else {
                this._map.set(charCode, value);
            }
        };
        CharacterClassifier.prototype.get = function (charCode) {
            if (charCode >= 0 && charCode < 256) {
                return this._asciiMap[charCode];
            }
            else {
                return (this._map.get(charCode) || this._defaultValue);
            }
        };
        return CharacterClassifier;
    }());
    exports.CharacterClassifier = CharacterClassifier;
    var CharacterSet = /** @class */ (function () {
        function CharacterSet() {
            this._actual = new CharacterClassifier(0 /* False */);
        }
        CharacterSet.prototype.add = function (charCode) {
            this._actual.set(charCode, 1 /* True */);
        };
        CharacterSet.prototype.has = function (charCode) {
            return (this._actual.get(charCode) === 1 /* True */);
        };
        return CharacterSet;
    }());
    exports.CharacterSet = CharacterSet;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[2/*vs/editor/common/core/position*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * A position in the editor.
     */
    var Position = /** @class */ (function () {
        function Position(lineNumber, column) {
            this.lineNumber = lineNumber;
            this.column = column;
        }
        /**
         * Create a new position from this position.
         *
         * @param newLineNumber new line number
         * @param newColumn new column
         */
        Position.prototype.with = function (newLineNumber, newColumn) {
            if (newLineNumber === void 0) { newLineNumber = this.lineNumber; }
            if (newColumn === void 0) { newColumn = this.column; }
            if (newLineNumber === this.lineNumber && newColumn === this.column) {
                return this;
            }
            else {
                return new Position(newLineNumber, newColumn);
            }
        };
        /**
         * Derive a new position from this position.
         *
         * @param deltaLineNumber line number delta
         * @param deltaColumn column delta
         */
        Position.prototype.delta = function (deltaLineNumber, deltaColumn) {
            if (deltaLineNumber === void 0) { deltaLineNumber = 0; }
            if (deltaColumn === void 0) { deltaColumn = 0; }
            return this.with(this.lineNumber + deltaLineNumber, this.column + deltaColumn);
        };
        /**
         * Test if this position equals other position
         */
        Position.prototype.equals = function (other) {
            return Position.equals(this, other);
        };
        /**
         * Test if position `a` equals position `b`
         */
        Position.equals = function (a, b) {
            if (!a && !b) {
                return true;
            }
            return (!!a &&
                !!b &&
                a.lineNumber === b.lineNumber &&
                a.column === b.column);
        };
        /**
         * Test if this position is before other position.
         * If the two positions are equal, the result will be false.
         */
        Position.prototype.isBefore = function (other) {
            return Position.isBefore(this, other);
        };
        /**
         * Test if position `a` is before position `b`.
         * If the two positions are equal, the result will be false.
         */
        Position.isBefore = function (a, b) {
            if (a.lineNumber < b.lineNumber) {
                return true;
            }
            if (b.lineNumber < a.lineNumber) {
                return false;
            }
            return a.column < b.column;
        };
        /**
         * Test if this position is before other position.
         * If the two positions are equal, the result will be true.
         */
        Position.prototype.isBeforeOrEqual = function (other) {
            return Position.isBeforeOrEqual(this, other);
        };
        /**
         * Test if position `a` is before position `b`.
         * If the two positions are equal, the result will be true.
         */
        Position.isBeforeOrEqual = function (a, b) {
            if (a.lineNumber < b.lineNumber) {
                return true;
            }
            if (b.lineNumber < a.lineNumber) {
                return false;
            }
            return a.column <= b.column;
        };
        /**
         * A function that compares positions, useful for sorting
         */
        Position.compare = function (a, b) {
            var aLineNumber = a.lineNumber | 0;
            var bLineNumber = b.lineNumber | 0;
            if (aLineNumber === bLineNumber) {
                var aColumn = a.column | 0;
                var bColumn = b.column | 0;
                return aColumn - bColumn;
            }
            return aLineNumber - bLineNumber;
        };
        /**
         * Clone this position.
         */
        Position.prototype.clone = function () {
            return new Position(this.lineNumber, this.column);
        };
        /**
         * Convert to a human-readable representation.
         */
        Position.prototype.toString = function () {
            return '(' + this.lineNumber + ',' + this.column + ')';
        };
        // ---
        /**
         * Create a `Position` from an `IPosition`.
         */
        Position.lift = function (pos) {
            return new Position(pos.lineNumber, pos.column);
        };
        /**
         * Test if `obj` is an `IPosition`.
         */
        Position.isIPosition = function (obj) {
            return (obj
                && (typeof obj.lineNumber === 'number')
                && (typeof obj.column === 'number'));
        };
        return Position;
    }());
    exports.Position = Position;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[4/*vs/editor/common/core/range*/], __M([1/*require*/,0/*exports*/,2/*vs/editor/common/core/position*/]), function (require, exports, position_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * A range in the editor. (startLineNumber,startColumn) is <= (endLineNumber,endColumn)
     */
    var Range = /** @class */ (function () {
        function Range(startLineNumber, startColumn, endLineNumber, endColumn) {
            if ((startLineNumber > endLineNumber) || (startLineNumber === endLineNumber && startColumn > endColumn)) {
                this.startLineNumber = endLineNumber;
                this.startColumn = endColumn;
                this.endLineNumber = startLineNumber;
                this.endColumn = startColumn;
            }
            else {
                this.startLineNumber = startLineNumber;
                this.startColumn = startColumn;
                this.endLineNumber = endLineNumber;
                this.endColumn = endColumn;
            }
        }
        /**
         * Test if this range is empty.
         */
        Range.prototype.isEmpty = function () {
            return Range.isEmpty(this);
        };
        /**
         * Test if `range` is empty.
         */
        Range.isEmpty = function (range) {
            return (range.startLineNumber === range.endLineNumber && range.startColumn === range.endColumn);
        };
        /**
         * Test if position is in this range. If the position is at the edges, will return true.
         */
        Range.prototype.containsPosition = function (position) {
            return Range.containsPosition(this, position);
        };
        /**
         * Test if `position` is in `range`. If the position is at the edges, will return true.
         */
        Range.containsPosition = function (range, position) {
            if (position.lineNumber < range.startLineNumber || position.lineNumber > range.endLineNumber) {
                return false;
            }
            if (position.lineNumber === range.startLineNumber && position.column < range.startColumn) {
                return false;
            }
            if (position.lineNumber === range.endLineNumber && position.column > range.endColumn) {
                return false;
            }
            return true;
        };
        /**
         * Test if range is in this range. If the range is equal to this range, will return true.
         */
        Range.prototype.containsRange = function (range) {
            return Range.containsRange(this, range);
        };
        /**
         * Test if `otherRange` is in `range`. If the ranges are equal, will return true.
         */
        Range.containsRange = function (range, otherRange) {
            if (otherRange.startLineNumber < range.startLineNumber || otherRange.endLineNumber < range.startLineNumber) {
                return false;
            }
            if (otherRange.startLineNumber > range.endLineNumber || otherRange.endLineNumber > range.endLineNumber) {
                return false;
            }
            if (otherRange.startLineNumber === range.startLineNumber && otherRange.startColumn < range.startColumn) {
                return false;
            }
            if (otherRange.endLineNumber === range.endLineNumber && otherRange.endColumn > range.endColumn) {
                return false;
            }
            return true;
        };
        /**
         * Test if `range` is strictly in this range. `range` must start after and end before this range for the result to be true.
         */
        Range.prototype.strictContainsRange = function (range) {
            return Range.strictContainsRange(this, range);
        };
        /**
         * Test if `otherRange` is strinctly in `range` (must start after, and end before). If the ranges are equal, will return false.
         */
        Range.strictContainsRange = function (range, otherRange) {
            if (otherRange.startLineNumber < range.startLineNumber || otherRange.endLineNumber < range.startLineNumber) {
                return false;
            }
            if (otherRange.startLineNumber > range.endLineNumber || otherRange.endLineNumber > range.endLineNumber) {
                return false;
            }
            if (otherRange.startLineNumber === range.startLineNumber && otherRange.startColumn <= range.startColumn) {
                return false;
            }
            if (otherRange.endLineNumber === range.endLineNumber && otherRange.endColumn >= range.endColumn) {
                return false;
            }
            return true;
        };
        /**
         * A reunion of the two ranges.
         * The smallest position will be used as the start point, and the largest one as the end point.
         */
        Range.prototype.plusRange = function (range) {
            return Range.plusRange(this, range);
        };
        /**
         * A reunion of the two ranges.
         * The smallest position will be used as the start point, and the largest one as the end point.
         */
        Range.plusRange = function (a, b) {
            var startLineNumber;
            var startColumn;
            var endLineNumber;
            var endColumn;
            if (b.startLineNumber < a.startLineNumber) {
                startLineNumber = b.startLineNumber;
                startColumn = b.startColumn;
            }
            else if (b.startLineNumber === a.startLineNumber) {
                startLineNumber = b.startLineNumber;
                startColumn = Math.min(b.startColumn, a.startColumn);
            }
            else {
                startLineNumber = a.startLineNumber;
                startColumn = a.startColumn;
            }
            if (b.endLineNumber > a.endLineNumber) {
                endLineNumber = b.endLineNumber;
                endColumn = b.endColumn;
            }
            else if (b.endLineNumber === a.endLineNumber) {
                endLineNumber = b.endLineNumber;
                endColumn = Math.max(b.endColumn, a.endColumn);
            }
            else {
                endLineNumber = a.endLineNumber;
                endColumn = a.endColumn;
            }
            return new Range(startLineNumber, startColumn, endLineNumber, endColumn);
        };
        /**
         * A intersection of the two ranges.
         */
        Range.prototype.intersectRanges = function (range) {
            return Range.intersectRanges(this, range);
        };
        /**
         * A intersection of the two ranges.
         */
        Range.intersectRanges = function (a, b) {
            var resultStartLineNumber = a.startLineNumber;
            var resultStartColumn = a.startColumn;
            var resultEndLineNumber = a.endLineNumber;
            var resultEndColumn = a.endColumn;
            var otherStartLineNumber = b.startLineNumber;
            var otherStartColumn = b.startColumn;
            var otherEndLineNumber = b.endLineNumber;
            var otherEndColumn = b.endColumn;
            if (resultStartLineNumber < otherStartLineNumber) {
                resultStartLineNumber = otherStartLineNumber;
                resultStartColumn = otherStartColumn;
            }
            else if (resultStartLineNumber === otherStartLineNumber) {
                resultStartColumn = Math.max(resultStartColumn, otherStartColumn);
            }
            if (resultEndLineNumber > otherEndLineNumber) {
                resultEndLineNumber = otherEndLineNumber;
                resultEndColumn = otherEndColumn;
            }
            else if (resultEndLineNumber === otherEndLineNumber) {
                resultEndColumn = Math.min(resultEndColumn, otherEndColumn);
            }
            // Check if selection is now empty
            if (resultStartLineNumber > resultEndLineNumber) {
                return null;
            }
            if (resultStartLineNumber === resultEndLineNumber && resultStartColumn > resultEndColumn) {
                return null;
            }
            return new Range(resultStartLineNumber, resultStartColumn, resultEndLineNumber, resultEndColumn);
        };
        /**
         * Test if this range equals other.
         */
        Range.prototype.equalsRange = function (other) {
            return Range.equalsRange(this, other);
        };
        /**
         * Test if range `a` equals `b`.
         */
        Range.equalsRange = function (a, b) {
            return (!!a &&
                !!b &&
                a.startLineNumber === b.startLineNumber &&
                a.startColumn === b.startColumn &&
                a.endLineNumber === b.endLineNumber &&
                a.endColumn === b.endColumn);
        };
        /**
         * Return the end position (which will be after or equal to the start position)
         */
        Range.prototype.getEndPosition = function () {
            return new position_1.Position(this.endLineNumber, this.endColumn);
        };
        /**
         * Return the start position (which will be before or equal to the end position)
         */
        Range.prototype.getStartPosition = function () {
            return new position_1.Position(this.startLineNumber, this.startColumn);
        };
        /**
         * Transform to a user presentable string representation.
         */
        Range.prototype.toString = function () {
            return '[' + this.startLineNumber + ',' + this.startColumn + ' -> ' + this.endLineNumber + ',' + this.endColumn + ']';
        };
        /**
         * Create a new range using this range's start position, and using endLineNumber and endColumn as the end position.
         */
        Range.prototype.setEndPosition = function (endLineNumber, endColumn) {
            return new Range(this.startLineNumber, this.startColumn, endLineNumber, endColumn);
        };
        /**
         * Create a new range using this range's end position, and using startLineNumber and startColumn as the start position.
         */
        Range.prototype.setStartPosition = function (startLineNumber, startColumn) {
            return new Range(startLineNumber, startColumn, this.endLineNumber, this.endColumn);
        };
        /**
         * Create a new empty range using this range's start position.
         */
        Range.prototype.collapseToStart = function () {
            return Range.collapseToStart(this);
        };
        /**
         * Create a new empty range using this range's start position.
         */
        Range.collapseToStart = function (range) {
            return new Range(range.startLineNumber, range.startColumn, range.startLineNumber, range.startColumn);
        };
        // ---
        Range.fromPositions = function (start, end) {
            if (end === void 0) { end = start; }
            return new Range(start.lineNumber, start.column, end.lineNumber, end.column);
        };
        Range.lift = function (range) {
            if (!range) {
                return null;
            }
            return new Range(range.startLineNumber, range.startColumn, range.endLineNumber, range.endColumn);
        };
        /**
         * Test if `obj` is an `IRange`.
         */
        Range.isIRange = function (obj) {
            return (obj
                && (typeof obj.startLineNumber === 'number')
                && (typeof obj.startColumn === 'number')
                && (typeof obj.endLineNumber === 'number')
                && (typeof obj.endColumn === 'number'));
        };
        /**
         * Test if the two ranges are touching in any way.
         */
        Range.areIntersectingOrTouching = function (a, b) {
            // Check if `a` is before `b`
            if (a.endLineNumber < b.startLineNumber || (a.endLineNumber === b.startLineNumber && a.endColumn < b.startColumn)) {
                return false;
            }
            // Check if `b` is before `a`
            if (b.endLineNumber < a.startLineNumber || (b.endLineNumber === a.startLineNumber && b.endColumn < a.startColumn)) {
                return false;
            }
            // These ranges must intersect
            return true;
        };
        /**
         * Test if the two ranges are intersecting. If the ranges are touching it returns true.
         */
        Range.areIntersecting = function (a, b) {
            // Check if `a` is before `b`
            if (a.endLineNumber < b.startLineNumber || (a.endLineNumber === b.startLineNumber && a.endColumn <= b.startColumn)) {
                return false;
            }
            // Check if `b` is before `a`
            if (b.endLineNumber < a.startLineNumber || (b.endLineNumber === a.startLineNumber && b.endColumn <= a.startColumn)) {
                return false;
            }
            // These ranges must intersect
            return true;
        };
        /**
         * A function that compares ranges, useful for sorting ranges
         * It will first compare ranges on the startPosition and then on the endPosition
         */
        Range.compareRangesUsingStarts = function (a, b) {
            if (a && b) {
                var aStartLineNumber = a.startLineNumber | 0;
                var bStartLineNumber = b.startLineNumber | 0;
                if (aStartLineNumber === bStartLineNumber) {
                    var aStartColumn = a.startColumn | 0;
                    var bStartColumn = b.startColumn | 0;
                    if (aStartColumn === bStartColumn) {
                        var aEndLineNumber = a.endLineNumber | 0;
                        var bEndLineNumber = b.endLineNumber | 0;
                        if (aEndLineNumber === bEndLineNumber) {
                            var aEndColumn = a.endColumn | 0;
                            var bEndColumn = b.endColumn | 0;
                            return aEndColumn - bEndColumn;
                        }
                        return aEndLineNumber - bEndLineNumber;
                    }
                    return aStartColumn - bStartColumn;
                }
                return aStartLineNumber - bStartLineNumber;
            }
            var aExists = (a ? 1 : 0);
            var bExists = (b ? 1 : 0);
            return aExists - bExists;
        };
        /**
         * A function that compares ranges, useful for sorting ranges
         * It will first compare ranges on the endPosition and then on the startPosition
         */
        Range.compareRangesUsingEnds = function (a, b) {
            if (a.endLineNumber === b.endLineNumber) {
                if (a.endColumn === b.endColumn) {
                    if (a.startLineNumber === b.startLineNumber) {
                        return a.startColumn - b.startColumn;
                    }
                    return a.startLineNumber - b.startLineNumber;
                }
                return a.endColumn - b.endColumn;
            }
            return a.endLineNumber - b.endLineNumber;
        };
        /**
         * Test if the range spans multiple lines.
         */
        Range.spansMultipleLines = function (range) {
            return range.endLineNumber > range.startLineNumber;
        };
        return Range;
    }());
    exports.Range = Range;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/













define(__m[23/*vs/editor/common/core/selection*/], __M([1/*require*/,0/*exports*/,2/*vs/editor/common/core/position*/,4/*vs/editor/common/core/range*/]), function (require, exports, position_1, range_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * A selection in the editor.
     * The selection is a range that has an orientation.
     */
    var Selection = /** @class */ (function (_super) {
        __extends(Selection, _super);
        function Selection(selectionStartLineNumber, selectionStartColumn, positionLineNumber, positionColumn) {
            var _this = _super.call(this, selectionStartLineNumber, selectionStartColumn, positionLineNumber, positionColumn) || this;
            _this.selectionStartLineNumber = selectionStartLineNumber;
            _this.selectionStartColumn = selectionStartColumn;
            _this.positionLineNumber = positionLineNumber;
            _this.positionColumn = positionColumn;
            return _this;
        }
        /**
         * Transform to a human-readable representation.
         */
        Selection.prototype.toString = function () {
            return '[' + this.selectionStartLineNumber + ',' + this.selectionStartColumn + ' -> ' + this.positionLineNumber + ',' + this.positionColumn + ']';
        };
        /**
         * Test if equals other selection.
         */
        Selection.prototype.equalsSelection = function (other) {
            return (Selection.selectionsEqual(this, other));
        };
        /**
         * Test if the two selections are equal.
         */
        Selection.selectionsEqual = function (a, b) {
            return (a.selectionStartLineNumber === b.selectionStartLineNumber &&
                a.selectionStartColumn === b.selectionStartColumn &&
                a.positionLineNumber === b.positionLineNumber &&
                a.positionColumn === b.positionColumn);
        };
        /**
         * Get directions (LTR or RTL).
         */
        Selection.prototype.getDirection = function () {
            if (this.selectionStartLineNumber === this.startLineNumber && this.selectionStartColumn === this.startColumn) {
                return 0 /* LTR */;
            }
            return 1 /* RTL */;
        };
        /**
         * Create a new selection with a different `positionLineNumber` and `positionColumn`.
         */
        Selection.prototype.setEndPosition = function (endLineNumber, endColumn) {
            if (this.getDirection() === 0 /* LTR */) {
                return new Selection(this.startLineNumber, this.startColumn, endLineNumber, endColumn);
            }
            return new Selection(endLineNumber, endColumn, this.startLineNumber, this.startColumn);
        };
        /**
         * Get the position at `positionLineNumber` and `positionColumn`.
         */
        Selection.prototype.getPosition = function () {
            return new position_1.Position(this.positionLineNumber, this.positionColumn);
        };
        /**
         * Create a new selection with a different `selectionStartLineNumber` and `selectionStartColumn`.
         */
        Selection.prototype.setStartPosition = function (startLineNumber, startColumn) {
            if (this.getDirection() === 0 /* LTR */) {
                return new Selection(startLineNumber, startColumn, this.endLineNumber, this.endColumn);
            }
            return new Selection(this.endLineNumber, this.endColumn, startLineNumber, startColumn);
        };
        // ----
        /**
         * Create a `Selection` from one or two positions
         */
        Selection.fromPositions = function (start, end) {
            if (end === void 0) { end = start; }
            return new Selection(start.lineNumber, start.column, end.lineNumber, end.column);
        };
        /**
         * Create a `Selection` from an `ISelection`.
         */
        Selection.liftSelection = function (sel) {
            return new Selection(sel.selectionStartLineNumber, sel.selectionStartColumn, sel.positionLineNumber, sel.positionColumn);
        };
        /**
         * `a` equals `b`.
         */
        Selection.selectionsArrEqual = function (a, b) {
            if (a && !b || !a && b) {
                return false;
            }
            if (!a && !b) {
                return true;
            }
            if (a.length !== b.length) {
                return false;
            }
            for (var i = 0, len = a.length; i < len; i++) {
                if (!this.selectionsEqual(a[i], b[i])) {
                    return false;
                }
            }
            return true;
        };
        /**
         * Test if `obj` is an `ISelection`.
         */
        Selection.isISelection = function (obj) {
            return (obj
                && (typeof obj.selectionStartLineNumber === 'number')
                && (typeof obj.selectionStartColumn === 'number')
                && (typeof obj.positionLineNumber === 'number')
                && (typeof obj.positionColumn === 'number'));
        };
        /**
         * Create with a direction.
         */
        Selection.createWithDirection = function (startLineNumber, startColumn, endLineNumber, endColumn, direction) {
            if (direction === 0 /* LTR */) {
                return new Selection(startLineNumber, startColumn, endLineNumber, endColumn);
            }
            return new Selection(endLineNumber, endColumn, startLineNumber, startColumn);
        };
        return Selection;
    }(range_1.Range));
    exports.Selection = Selection;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[24/*vs/editor/common/core/token*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var Token = /** @class */ (function () {
        function Token(offset, type, language) {
            this.offset = offset | 0; // @perf
            this.type = type;
            this.language = language;
        }
        Token.prototype.toString = function () {
            return '(' + this.offset + ', ' + this.type + ')';
        };
        return Token;
    }());
    exports.Token = Token;
    var TokenizationResult = /** @class */ (function () {
        function TokenizationResult(tokens, endState) {
            this.tokens = tokens;
            this.endState = endState;
        }
        return TokenizationResult;
    }());
    exports.TokenizationResult = TokenizationResult;
    var TokenizationResult2 = /** @class */ (function () {
        function TokenizationResult2(tokens, endState) {
            this.tokens = tokens;
            this.endState = endState;
        }
        return TokenizationResult2;
    }());
    exports.TokenizationResult2 = TokenizationResult2;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[25/*vs/editor/common/diff/diffComputer*/], __M([1/*require*/,0/*exports*/,10/*vs/base/common/diff/diff*/,15/*vs/base/common/strings*/]), function (require, exports, diff_1, strings) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var MINIMUM_MATCHING_CHARACTER_LENGTH = 3;
    function computeDiff(originalSequence, modifiedSequence, continueProcessingPredicate, pretty) {
        var diffAlgo = new diff_1.LcsDiff(originalSequence, modifiedSequence, continueProcessingPredicate);
        return diffAlgo.ComputeDiff(pretty);
    }
    var LineSequence = /** @class */ (function () {
        function LineSequence(lines) {
            var startColumns = [];
            var endColumns = [];
            for (var i = 0, length_1 = lines.length; i < length_1; i++) {
                startColumns[i] = getFirstNonBlankColumn(lines[i], 1);
                endColumns[i] = getLastNonBlankColumn(lines[i], 1);
            }
            this.lines = lines;
            this._startColumns = startColumns;
            this._endColumns = endColumns;
        }
        LineSequence.prototype.getElements = function () {
            var elements = [];
            for (var i = 0, len = this.lines.length; i < len; i++) {
                elements[i] = this.lines[i].substring(this._startColumns[i] - 1, this._endColumns[i] - 1);
            }
            return elements;
        };
        LineSequence.prototype.getStartLineNumber = function (i) {
            return i + 1;
        };
        LineSequence.prototype.getEndLineNumber = function (i) {
            return i + 1;
        };
        LineSequence.prototype.createCharSequence = function (shouldIgnoreTrimWhitespace, startIndex, endIndex) {
            var charCodes = [];
            var lineNumbers = [];
            var columns = [];
            var len = 0;
            for (var index = startIndex; index <= endIndex; index++) {
                var lineContent = this.lines[index];
                var startColumn = (shouldIgnoreTrimWhitespace ? this._startColumns[index] : 1);
                var endColumn = (shouldIgnoreTrimWhitespace ? this._endColumns[index] : lineContent.length + 1);
                for (var col = startColumn; col < endColumn; col++) {
                    charCodes[len] = lineContent.charCodeAt(col - 1);
                    lineNumbers[len] = index + 1;
                    columns[len] = col;
                    len++;
                }
            }
            return new CharSequence(charCodes, lineNumbers, columns);
        };
        return LineSequence;
    }());
    var CharSequence = /** @class */ (function () {
        function CharSequence(charCodes, lineNumbers, columns) {
            this._charCodes = charCodes;
            this._lineNumbers = lineNumbers;
            this._columns = columns;
        }
        CharSequence.prototype.getElements = function () {
            return this._charCodes;
        };
        CharSequence.prototype.getStartLineNumber = function (i) {
            return this._lineNumbers[i];
        };
        CharSequence.prototype.getStartColumn = function (i) {
            return this._columns[i];
        };
        CharSequence.prototype.getEndLineNumber = function (i) {
            return this._lineNumbers[i];
        };
        CharSequence.prototype.getEndColumn = function (i) {
            return this._columns[i] + 1;
        };
        return CharSequence;
    }());
    var CharChange = /** @class */ (function () {
        function CharChange(originalStartLineNumber, originalStartColumn, originalEndLineNumber, originalEndColumn, modifiedStartLineNumber, modifiedStartColumn, modifiedEndLineNumber, modifiedEndColumn) {
            this.originalStartLineNumber = originalStartLineNumber;
            this.originalStartColumn = originalStartColumn;
            this.originalEndLineNumber = originalEndLineNumber;
            this.originalEndColumn = originalEndColumn;
            this.modifiedStartLineNumber = modifiedStartLineNumber;
            this.modifiedStartColumn = modifiedStartColumn;
            this.modifiedEndLineNumber = modifiedEndLineNumber;
            this.modifiedEndColumn = modifiedEndColumn;
        }
        CharChange.createFromDiffChange = function (diffChange, originalCharSequence, modifiedCharSequence) {
            var originalStartLineNumber;
            var originalStartColumn;
            var originalEndLineNumber;
            var originalEndColumn;
            var modifiedStartLineNumber;
            var modifiedStartColumn;
            var modifiedEndLineNumber;
            var modifiedEndColumn;
            if (diffChange.originalLength === 0) {
                originalStartLineNumber = 0;
                originalStartColumn = 0;
                originalEndLineNumber = 0;
                originalEndColumn = 0;
            }
            else {
                originalStartLineNumber = originalCharSequence.getStartLineNumber(diffChange.originalStart);
                originalStartColumn = originalCharSequence.getStartColumn(diffChange.originalStart);
                originalEndLineNumber = originalCharSequence.getEndLineNumber(diffChange.originalStart + diffChange.originalLength - 1);
                originalEndColumn = originalCharSequence.getEndColumn(diffChange.originalStart + diffChange.originalLength - 1);
            }
            if (diffChange.modifiedLength === 0) {
                modifiedStartLineNumber = 0;
                modifiedStartColumn = 0;
                modifiedEndLineNumber = 0;
                modifiedEndColumn = 0;
            }
            else {
                modifiedStartLineNumber = modifiedCharSequence.getStartLineNumber(diffChange.modifiedStart);
                modifiedStartColumn = modifiedCharSequence.getStartColumn(diffChange.modifiedStart);
                modifiedEndLineNumber = modifiedCharSequence.getEndLineNumber(diffChange.modifiedStart + diffChange.modifiedLength - 1);
                modifiedEndColumn = modifiedCharSequence.getEndColumn(diffChange.modifiedStart + diffChange.modifiedLength - 1);
            }
            return new CharChange(originalStartLineNumber, originalStartColumn, originalEndLineNumber, originalEndColumn, modifiedStartLineNumber, modifiedStartColumn, modifiedEndLineNumber, modifiedEndColumn);
        };
        return CharChange;
    }());
    function postProcessCharChanges(rawChanges) {
        if (rawChanges.length <= 1) {
            return rawChanges;
        }
        var result = [rawChanges[0]];
        var prevChange = result[0];
        for (var i = 1, len = rawChanges.length; i < len; i++) {
            var currChange = rawChanges[i];
            var originalMatchingLength = currChange.originalStart - (prevChange.originalStart + prevChange.originalLength);
            var modifiedMatchingLength = currChange.modifiedStart - (prevChange.modifiedStart + prevChange.modifiedLength);
            // Both of the above should be equal, but the continueProcessingPredicate may prevent this from being true
            var matchingLength = Math.min(originalMatchingLength, modifiedMatchingLength);
            if (matchingLength < MINIMUM_MATCHING_CHARACTER_LENGTH) {
                // Merge the current change into the previous one
                prevChange.originalLength = (currChange.originalStart + currChange.originalLength) - prevChange.originalStart;
                prevChange.modifiedLength = (currChange.modifiedStart + currChange.modifiedLength) - prevChange.modifiedStart;
            }
            else {
                // Add the current change
                result.push(currChange);
                prevChange = currChange;
            }
        }
        return result;
    }
    var LineChange = /** @class */ (function () {
        function LineChange(originalStartLineNumber, originalEndLineNumber, modifiedStartLineNumber, modifiedEndLineNumber, charChanges) {
            this.originalStartLineNumber = originalStartLineNumber;
            this.originalEndLineNumber = originalEndLineNumber;
            this.modifiedStartLineNumber = modifiedStartLineNumber;
            this.modifiedEndLineNumber = modifiedEndLineNumber;
            this.charChanges = charChanges;
        }
        LineChange.createFromDiffResult = function (shouldIgnoreTrimWhitespace, diffChange, originalLineSequence, modifiedLineSequence, continueCharDiff, shouldComputeCharChanges, shouldPostProcessCharChanges) {
            var originalStartLineNumber;
            var originalEndLineNumber;
            var modifiedStartLineNumber;
            var modifiedEndLineNumber;
            var charChanges = undefined;
            if (diffChange.originalLength === 0) {
                originalStartLineNumber = originalLineSequence.getStartLineNumber(diffChange.originalStart) - 1;
                originalEndLineNumber = 0;
            }
            else {
                originalStartLineNumber = originalLineSequence.getStartLineNumber(diffChange.originalStart);
                originalEndLineNumber = originalLineSequence.getEndLineNumber(diffChange.originalStart + diffChange.originalLength - 1);
            }
            if (diffChange.modifiedLength === 0) {
                modifiedStartLineNumber = modifiedLineSequence.getStartLineNumber(diffChange.modifiedStart) - 1;
                modifiedEndLineNumber = 0;
            }
            else {
                modifiedStartLineNumber = modifiedLineSequence.getStartLineNumber(diffChange.modifiedStart);
                modifiedEndLineNumber = modifiedLineSequence.getEndLineNumber(diffChange.modifiedStart + diffChange.modifiedLength - 1);
            }
            if (shouldComputeCharChanges && diffChange.originalLength > 0 && diffChange.originalLength < 20 && diffChange.modifiedLength > 0 && diffChange.modifiedLength < 20 && continueCharDiff()) {
                // Compute character changes for diff chunks of at most 20 lines...
                var originalCharSequence = originalLineSequence.createCharSequence(shouldIgnoreTrimWhitespace, diffChange.originalStart, diffChange.originalStart + diffChange.originalLength - 1);
                var modifiedCharSequence = modifiedLineSequence.createCharSequence(shouldIgnoreTrimWhitespace, diffChange.modifiedStart, diffChange.modifiedStart + diffChange.modifiedLength - 1);
                var rawChanges = computeDiff(originalCharSequence, modifiedCharSequence, continueCharDiff, true).changes;
                if (shouldPostProcessCharChanges) {
                    rawChanges = postProcessCharChanges(rawChanges);
                }
                charChanges = [];
                for (var i = 0, length_2 = rawChanges.length; i < length_2; i++) {
                    charChanges.push(CharChange.createFromDiffChange(rawChanges[i], originalCharSequence, modifiedCharSequence));
                }
            }
            return new LineChange(originalStartLineNumber, originalEndLineNumber, modifiedStartLineNumber, modifiedEndLineNumber, charChanges);
        };
        return LineChange;
    }());
    var DiffComputer = /** @class */ (function () {
        function DiffComputer(originalLines, modifiedLines, opts) {
            this.shouldComputeCharChanges = opts.shouldComputeCharChanges;
            this.shouldPostProcessCharChanges = opts.shouldPostProcessCharChanges;
            this.shouldIgnoreTrimWhitespace = opts.shouldIgnoreTrimWhitespace;
            this.shouldMakePrettyDiff = opts.shouldMakePrettyDiff;
            this.originalLines = originalLines;
            this.modifiedLines = modifiedLines;
            this.original = new LineSequence(originalLines);
            this.modified = new LineSequence(modifiedLines);
            this.continueLineDiff = createContinueProcessingPredicate(opts.maxComputationTime);
            this.continueCharDiff = createContinueProcessingPredicate(opts.maxComputationTime === 0 ? 0 : Math.min(opts.maxComputationTime, 5000)); // never run after 5s for character changes...
        }
        DiffComputer.prototype.computeDiff = function () {
            if (this.original.lines.length === 1 && this.original.lines[0].length === 0) {
                // empty original => fast path
                return {
                    quitEarly: false,
                    changes: [{
                            originalStartLineNumber: 1,
                            originalEndLineNumber: 1,
                            modifiedStartLineNumber: 1,
                            modifiedEndLineNumber: this.modified.lines.length,
                            charChanges: [{
                                    modifiedEndColumn: 0,
                                    modifiedEndLineNumber: 0,
                                    modifiedStartColumn: 0,
                                    modifiedStartLineNumber: 0,
                                    originalEndColumn: 0,
                                    originalEndLineNumber: 0,
                                    originalStartColumn: 0,
                                    originalStartLineNumber: 0
                                }]
                        }]
                };
            }
            if (this.modified.lines.length === 1 && this.modified.lines[0].length === 0) {
                // empty modified => fast path
                return {
                    quitEarly: false,
                    changes: [{
                            originalStartLineNumber: 1,
                            originalEndLineNumber: this.original.lines.length,
                            modifiedStartLineNumber: 1,
                            modifiedEndLineNumber: 1,
                            charChanges: [{
                                    modifiedEndColumn: 0,
                                    modifiedEndLineNumber: 0,
                                    modifiedStartColumn: 0,
                                    modifiedStartLineNumber: 0,
                                    originalEndColumn: 0,
                                    originalEndLineNumber: 0,
                                    originalStartColumn: 0,
                                    originalStartLineNumber: 0
                                }]
                        }]
                };
            }
            var diffResult = computeDiff(this.original, this.modified, this.continueLineDiff, this.shouldMakePrettyDiff);
            var rawChanges = diffResult.changes;
            var quitEarly = diffResult.quitEarly;
            // The diff is always computed with ignoring trim whitespace
            // This ensures we get the prettiest diff
            if (this.shouldIgnoreTrimWhitespace) {
                var lineChanges = [];
                for (var i = 0, length_3 = rawChanges.length; i < length_3; i++) {
                    lineChanges.push(LineChange.createFromDiffResult(this.shouldIgnoreTrimWhitespace, rawChanges[i], this.original, this.modified, this.continueCharDiff, this.shouldComputeCharChanges, this.shouldPostProcessCharChanges));
                }
                return {
                    quitEarly: quitEarly,
                    changes: lineChanges
                };
            }
            // Need to post-process and introduce changes where the trim whitespace is different
            // Note that we are looping starting at -1 to also cover the lines before the first change
            var result = [];
            var originalLineIndex = 0;
            var modifiedLineIndex = 0;
            for (var i = -1 /* !!!! */, len = rawChanges.length; i < len; i++) {
                var nextChange = (i + 1 < len ? rawChanges[i + 1] : null);
                var originalStop = (nextChange ? nextChange.originalStart : this.originalLines.length);
                var modifiedStop = (nextChange ? nextChange.modifiedStart : this.modifiedLines.length);
                while (originalLineIndex < originalStop && modifiedLineIndex < modifiedStop) {
                    var originalLine = this.originalLines[originalLineIndex];
                    var modifiedLine = this.modifiedLines[modifiedLineIndex];
                    if (originalLine !== modifiedLine) {
                        // These lines differ only in trim whitespace
                        // Check the leading whitespace
                        {
                            var originalStartColumn = getFirstNonBlankColumn(originalLine, 1);
                            var modifiedStartColumn = getFirstNonBlankColumn(modifiedLine, 1);
                            while (originalStartColumn > 1 && modifiedStartColumn > 1) {
                                var originalChar = originalLine.charCodeAt(originalStartColumn - 2);
                                var modifiedChar = modifiedLine.charCodeAt(modifiedStartColumn - 2);
                                if (originalChar !== modifiedChar) {
                                    break;
                                }
                                originalStartColumn--;
                                modifiedStartColumn--;
                            }
                            if (originalStartColumn > 1 || modifiedStartColumn > 1) {
                                this._pushTrimWhitespaceCharChange(result, originalLineIndex + 1, 1, originalStartColumn, modifiedLineIndex + 1, 1, modifiedStartColumn);
                            }
                        }
                        // Check the trailing whitespace
                        {
                            var originalEndColumn = getLastNonBlankColumn(originalLine, 1);
                            var modifiedEndColumn = getLastNonBlankColumn(modifiedLine, 1);
                            var originalMaxColumn = originalLine.length + 1;
                            var modifiedMaxColumn = modifiedLine.length + 1;
                            while (originalEndColumn < originalMaxColumn && modifiedEndColumn < modifiedMaxColumn) {
                                var originalChar = originalLine.charCodeAt(originalEndColumn - 1);
                                var modifiedChar = originalLine.charCodeAt(modifiedEndColumn - 1);
                                if (originalChar !== modifiedChar) {
                                    break;
                                }
                                originalEndColumn++;
                                modifiedEndColumn++;
                            }
                            if (originalEndColumn < originalMaxColumn || modifiedEndColumn < modifiedMaxColumn) {
                                this._pushTrimWhitespaceCharChange(result, originalLineIndex + 1, originalEndColumn, originalMaxColumn, modifiedLineIndex + 1, modifiedEndColumn, modifiedMaxColumn);
                            }
                        }
                    }
                    originalLineIndex++;
                    modifiedLineIndex++;
                }
                if (nextChange) {
                    // Emit the actual change
                    result.push(LineChange.createFromDiffResult(this.shouldIgnoreTrimWhitespace, nextChange, this.original, this.modified, this.continueCharDiff, this.shouldComputeCharChanges, this.shouldPostProcessCharChanges));
                    originalLineIndex += nextChange.originalLength;
                    modifiedLineIndex += nextChange.modifiedLength;
                }
            }
            return {
                quitEarly: quitEarly,
                changes: result
            };
        };
        DiffComputer.prototype._pushTrimWhitespaceCharChange = function (result, originalLineNumber, originalStartColumn, originalEndColumn, modifiedLineNumber, modifiedStartColumn, modifiedEndColumn) {
            if (this._mergeTrimWhitespaceCharChange(result, originalLineNumber, originalStartColumn, originalEndColumn, modifiedLineNumber, modifiedStartColumn, modifiedEndColumn)) {
                // Merged into previous
                return;
            }
            var charChanges = undefined;
            if (this.shouldComputeCharChanges) {
                charChanges = [new CharChange(originalLineNumber, originalStartColumn, originalLineNumber, originalEndColumn, modifiedLineNumber, modifiedStartColumn, modifiedLineNumber, modifiedEndColumn)];
            }
            result.push(new LineChange(originalLineNumber, originalLineNumber, modifiedLineNumber, modifiedLineNumber, charChanges));
        };
        DiffComputer.prototype._mergeTrimWhitespaceCharChange = function (result, originalLineNumber, originalStartColumn, originalEndColumn, modifiedLineNumber, modifiedStartColumn, modifiedEndColumn) {
            var len = result.length;
            if (len === 0) {
                return false;
            }
            var prevChange = result[len - 1];
            if (prevChange.originalEndLineNumber === 0 || prevChange.modifiedEndLineNumber === 0) {
                // Don't merge with inserts/deletes
                return false;
            }
            if (prevChange.originalEndLineNumber + 1 === originalLineNumber && prevChange.modifiedEndLineNumber + 1 === modifiedLineNumber) {
                prevChange.originalEndLineNumber = originalLineNumber;
                prevChange.modifiedEndLineNumber = modifiedLineNumber;
                if (this.shouldComputeCharChanges && prevChange.charChanges) {
                    prevChange.charChanges.push(new CharChange(originalLineNumber, originalStartColumn, originalLineNumber, originalEndColumn, modifiedLineNumber, modifiedStartColumn, modifiedLineNumber, modifiedEndColumn));
                }
                return true;
            }
            return false;
        };
        return DiffComputer;
    }());
    exports.DiffComputer = DiffComputer;
    function getFirstNonBlankColumn(txt, defaultValue) {
        var r = strings.firstNonWhitespaceIndex(txt);
        if (r === -1) {
            return defaultValue;
        }
        return r + 1;
    }
    function getLastNonBlankColumn(txt, defaultValue) {
        var r = strings.lastNonWhitespaceIndex(txt);
        if (r === -1) {
            return defaultValue;
        }
        return r + 2;
    }
    function createContinueProcessingPredicate(maximumRuntime) {
        if (maximumRuntime === 0) {
            return function () { return true; };
        }
        var startTime = Date.now();
        return function () {
            return Date.now() - startTime < maximumRuntime;
        };
    }
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[26/*vs/editor/common/model/wordHelper*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    exports.USUAL_WORD_SEPARATORS = '`~!@#$%^&*()-=+[{]}\\|;:\'",.<>/?';
    /**
     * Create a word definition regular expression based on default word separators.
     * Optionally provide allowed separators that should be included in words.
     *
     * The default would look like this:
     * /(-?\d*\.\d\w*)|([^\`\~\!\@\#\$\%\^\&\*\(\)\-\=\+\[\{\]\}\\\|\;\:\'\"\,\.\<\>\/\?\s]+)/g
     */
    function createWordRegExp(allowInWords) {
        if (allowInWords === void 0) { allowInWords = ''; }
        var source = '(-?\\d*\\.\\d\\w*)|([^';
        for (var _i = 0, USUAL_WORD_SEPARATORS_1 = exports.USUAL_WORD_SEPARATORS; _i < USUAL_WORD_SEPARATORS_1.length; _i++) {
            var sep = USUAL_WORD_SEPARATORS_1[_i];
            if (allowInWords.indexOf(sep) >= 0) {
                continue;
            }
            source += '\\' + sep;
        }
        source += '\\s]+)';
        return new RegExp(source, 'g');
    }
    // catches numbers (including floating numbers) in the first group, and alphanum in the second
    exports.DEFAULT_WORD_REGEXP = createWordRegExp();
    function ensureValidWordDefinition(wordDefinition) {
        var result = exports.DEFAULT_WORD_REGEXP;
        if (wordDefinition && (wordDefinition instanceof RegExp)) {
            if (!wordDefinition.global) {
                var flags = 'g';
                if (wordDefinition.ignoreCase) {
                    flags += 'i';
                }
                if (wordDefinition.multiline) {
                    flags += 'm';
                }
                if (wordDefinition.unicode) {
                    flags += 'u';
                }
                result = new RegExp(wordDefinition.source, flags);
            }
            else {
                result = wordDefinition;
            }
        }
        result.lastIndex = 0;
        return result;
    }
    exports.ensureValidWordDefinition = ensureValidWordDefinition;
    function getWordAtPosFast(column, wordDefinition, text, textOffset) {
        // find whitespace enclosed text around column and match from there
        var pos = column - 1 - textOffset;
        var start = text.lastIndexOf(' ', pos - 1) + 1;
        wordDefinition.lastIndex = start;
        var match;
        while (match = wordDefinition.exec(text)) {
            var matchIndex = match.index || 0;
            if (matchIndex <= pos && wordDefinition.lastIndex >= pos) {
                return {
                    word: match[0],
                    startColumn: textOffset + 1 + matchIndex,
                    endColumn: textOffset + 1 + wordDefinition.lastIndex
                };
            }
        }
        return null;
    }
    function getWordAtPosSlow(column, wordDefinition, text, textOffset) {
        // matches all words starting at the beginning
        // of the input until it finds a match that encloses
        // the desired column. slow but correct
        var pos = column - 1 - textOffset;
        wordDefinition.lastIndex = 0;
        var match;
        while (match = wordDefinition.exec(text)) {
            var matchIndex = match.index || 0;
            if (matchIndex > pos) {
                // |nW -> matched only after the pos
                return null;
            }
            else if (wordDefinition.lastIndex >= pos) {
                // W|W -> match encloses pos
                return {
                    word: match[0],
                    startColumn: textOffset + 1 + matchIndex,
                    endColumn: textOffset + 1 + wordDefinition.lastIndex
                };
            }
        }
        return null;
    }
    function getWordAtText(column, wordDefinition, text, textOffset) {
        // if `words` can contain whitespace character we have to use the slow variant
        // otherwise we use the fast variant of finding a word
        wordDefinition.lastIndex = 0;
        var match = wordDefinition.exec(text);
        if (!match) {
            return null;
        }
        // todo@joh the `match` could already be the (first) word
        var ret = match[0].indexOf(' ') >= 0
            // did match a word which contains a space character -> use slow word find
            ? getWordAtPosSlow(column, wordDefinition, text, textOffset)
            // sane word definition -> use fast word find
            : getWordAtPosFast(column, wordDefinition, text, textOffset);
        // both (getWordAtPosFast and getWordAtPosSlow) leave the wordDefinition-RegExp
        // in an undefined state and to not confuse other users of the wordDefinition
        // we reset the lastIndex
        wordDefinition.lastIndex = 0;
        return ret;
    }
    exports.getWordAtText = getWordAtText;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[27/*vs/editor/common/modes/linkComputer*/], __M([1/*require*/,0/*exports*/,20/*vs/editor/common/core/characterClassifier*/]), function (require, exports, characterClassifier_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var Uint8Matrix = /** @class */ (function () {
        function Uint8Matrix(rows, cols, defaultValue) {
            var data = new Uint8Array(rows * cols);
            for (var i = 0, len = rows * cols; i < len; i++) {
                data[i] = defaultValue;
            }
            this._data = data;
            this.rows = rows;
            this.cols = cols;
        }
        Uint8Matrix.prototype.get = function (row, col) {
            return this._data[row * this.cols + col];
        };
        Uint8Matrix.prototype.set = function (row, col, value) {
            this._data[row * this.cols + col] = value;
        };
        return Uint8Matrix;
    }());
    exports.Uint8Matrix = Uint8Matrix;
    var StateMachine = /** @class */ (function () {
        function StateMachine(edges) {
            var maxCharCode = 0;
            var maxState = 0 /* Invalid */;
            for (var i = 0, len = edges.length; i < len; i++) {
                var _a = edges[i], from = _a[0], chCode = _a[1], to = _a[2];
                if (chCode > maxCharCode) {
                    maxCharCode = chCode;
                }
                if (from > maxState) {
                    maxState = from;
                }
                if (to > maxState) {
                    maxState = to;
                }
            }
            maxCharCode++;
            maxState++;
            var states = new Uint8Matrix(maxState, maxCharCode, 0 /* Invalid */);
            for (var i = 0, len = edges.length; i < len; i++) {
                var _b = edges[i], from = _b[0], chCode = _b[1], to = _b[2];
                states.set(from, chCode, to);
            }
            this._states = states;
            this._maxCharCode = maxCharCode;
        }
        StateMachine.prototype.nextState = function (currentState, chCode) {
            if (chCode < 0 || chCode >= this._maxCharCode) {
                return 0 /* Invalid */;
            }
            return this._states.get(currentState, chCode);
        };
        return StateMachine;
    }());
    exports.StateMachine = StateMachine;
    // State machine for http:// or https:// or file://
    var _stateMachine = null;
    function getStateMachine() {
        if (_stateMachine === null) {
            _stateMachine = new StateMachine([
                [1 /* Start */, 104 /* h */, 2 /* H */],
                [1 /* Start */, 72 /* H */, 2 /* H */],
                [1 /* Start */, 102 /* f */, 6 /* F */],
                [1 /* Start */, 70 /* F */, 6 /* F */],
                [2 /* H */, 116 /* t */, 3 /* HT */],
                [2 /* H */, 84 /* T */, 3 /* HT */],
                [3 /* HT */, 116 /* t */, 4 /* HTT */],
                [3 /* HT */, 84 /* T */, 4 /* HTT */],
                [4 /* HTT */, 112 /* p */, 5 /* HTTP */],
                [4 /* HTT */, 80 /* P */, 5 /* HTTP */],
                [5 /* HTTP */, 115 /* s */, 9 /* BeforeColon */],
                [5 /* HTTP */, 83 /* S */, 9 /* BeforeColon */],
                [5 /* HTTP */, 58 /* Colon */, 10 /* AfterColon */],
                [6 /* F */, 105 /* i */, 7 /* FI */],
                [6 /* F */, 73 /* I */, 7 /* FI */],
                [7 /* FI */, 108 /* l */, 8 /* FIL */],
                [7 /* FI */, 76 /* L */, 8 /* FIL */],
                [8 /* FIL */, 101 /* e */, 9 /* BeforeColon */],
                [8 /* FIL */, 69 /* E */, 9 /* BeforeColon */],
                [9 /* BeforeColon */, 58 /* Colon */, 10 /* AfterColon */],
                [10 /* AfterColon */, 47 /* Slash */, 11 /* AlmostThere */],
                [11 /* AlmostThere */, 47 /* Slash */, 12 /* End */],
            ]);
        }
        return _stateMachine;
    }
    var _classifier = null;
    function getClassifier() {
        if (_classifier === null) {
            _classifier = new characterClassifier_1.CharacterClassifier(0 /* None */);
            var FORCE_TERMINATION_CHARACTERS = ' \t<>\'\"、。｡､，．：；？！＠＃＄％＆＊‘“〈《「『【〔（［｛｢｣｝］）〕】』」》〉”’｀～…';
            for (var i = 0; i < FORCE_TERMINATION_CHARACTERS.length; i++) {
                _classifier.set(FORCE_TERMINATION_CHARACTERS.charCodeAt(i), 1 /* ForceTermination */);
            }
            var CANNOT_END_WITH_CHARACTERS = '.,;';
            for (var i = 0; i < CANNOT_END_WITH_CHARACTERS.length; i++) {
                _classifier.set(CANNOT_END_WITH_CHARACTERS.charCodeAt(i), 2 /* CannotEndIn */);
            }
        }
        return _classifier;
    }
    var LinkComputer = /** @class */ (function () {
        function LinkComputer() {
        }
        LinkComputer._createLink = function (classifier, line, lineNumber, linkBeginIndex, linkEndIndex) {
            // Do not allow to end link in certain characters...
            var lastIncludedCharIndex = linkEndIndex - 1;
            do {
                var chCode = line.charCodeAt(lastIncludedCharIndex);
                var chClass = classifier.get(chCode);
                if (chClass !== 2 /* CannotEndIn */) {
                    break;
                }
                lastIncludedCharIndex--;
            } while (lastIncludedCharIndex > linkBeginIndex);
            // Handle links enclosed in parens, square brackets and curlys.
            if (linkBeginIndex > 0) {
                var charCodeBeforeLink = line.charCodeAt(linkBeginIndex - 1);
                var lastCharCodeInLink = line.charCodeAt(lastIncludedCharIndex);
                if ((charCodeBeforeLink === 40 /* OpenParen */ && lastCharCodeInLink === 41 /* CloseParen */)
                    || (charCodeBeforeLink === 91 /* OpenSquareBracket */ && lastCharCodeInLink === 93 /* CloseSquareBracket */)
                    || (charCodeBeforeLink === 123 /* OpenCurlyBrace */ && lastCharCodeInLink === 125 /* CloseCurlyBrace */)) {
                    // Do not end in ) if ( is before the link start
                    // Do not end in ] if [ is before the link start
                    // Do not end in } if { is before the link start
                    lastIncludedCharIndex--;
                }
            }
            return {
                range: {
                    startLineNumber: lineNumber,
                    startColumn: linkBeginIndex + 1,
                    endLineNumber: lineNumber,
                    endColumn: lastIncludedCharIndex + 2
                },
                url: line.substring(linkBeginIndex, lastIncludedCharIndex + 1)
            };
        };
        LinkComputer.computeLinks = function (model, stateMachine) {
            if (stateMachine === void 0) { stateMachine = getStateMachine(); }
            var classifier = getClassifier();
            var result = [];
            for (var i = 1, lineCount = model.getLineCount(); i <= lineCount; i++) {
                var line = model.getLineContent(i);
                var len = line.length;
                var j = 0;
                var linkBeginIndex = 0;
                var linkBeginChCode = 0;
                var state = 1 /* Start */;
                var hasOpenParens = false;
                var hasOpenSquareBracket = false;
                var hasOpenCurlyBracket = false;
                while (j < len) {
                    var resetStateMachine = false;
                    var chCode = line.charCodeAt(j);
                    if (state === 13 /* Accept */) {
                        var chClass = void 0;
                        switch (chCode) {
                            case 40 /* OpenParen */:
                                hasOpenParens = true;
                                chClass = 0 /* None */;
                                break;
                            case 41 /* CloseParen */:
                                chClass = (hasOpenParens ? 0 /* None */ : 1 /* ForceTermination */);
                                break;
                            case 91 /* OpenSquareBracket */:
                                hasOpenSquareBracket = true;
                                chClass = 0 /* None */;
                                break;
                            case 93 /* CloseSquareBracket */:
                                chClass = (hasOpenSquareBracket ? 0 /* None */ : 1 /* ForceTermination */);
                                break;
                            case 123 /* OpenCurlyBrace */:
                                hasOpenCurlyBracket = true;
                                chClass = 0 /* None */;
                                break;
                            case 125 /* CloseCurlyBrace */:
                                chClass = (hasOpenCurlyBracket ? 0 /* None */ : 1 /* ForceTermination */);
                                break;
                            /* The following three rules make it that ' or " or ` are allowed inside links if the link began with a different one */
                            case 39 /* SingleQuote */:
                                chClass = (linkBeginChCode === 34 /* DoubleQuote */ || linkBeginChCode === 96 /* BackTick */) ? 0 /* None */ : 1 /* ForceTermination */;
                                break;
                            case 34 /* DoubleQuote */:
                                chClass = (linkBeginChCode === 39 /* SingleQuote */ || linkBeginChCode === 96 /* BackTick */) ? 0 /* None */ : 1 /* ForceTermination */;
                                break;
                            case 96 /* BackTick */:
                                chClass = (linkBeginChCode === 39 /* SingleQuote */ || linkBeginChCode === 34 /* DoubleQuote */) ? 0 /* None */ : 1 /* ForceTermination */;
                                break;
                            case 42 /* Asterisk */:
                                // `*` terminates a link if the link began with `*`
                                chClass = (linkBeginChCode === 42 /* Asterisk */) ? 1 /* ForceTermination */ : 0 /* None */;
                                break;
                            case 124 /* Pipe */:
                                // `|` terminates a link if the link began with `|`
                                chClass = (linkBeginChCode === 124 /* Pipe */) ? 1 /* ForceTermination */ : 0 /* None */;
                                break;
                            default:
                                chClass = classifier.get(chCode);
                        }
                        // Check if character terminates link
                        if (chClass === 1 /* ForceTermination */) {
                            result.push(LinkComputer._createLink(classifier, line, i, linkBeginIndex, j));
                            resetStateMachine = true;
                        }
                    }
                    else if (state === 12 /* End */) {
                        var chClass = void 0;
                        if (chCode === 91 /* OpenSquareBracket */) {
                            // Allow for the authority part to contain ipv6 addresses which contain [ and ]
                            hasOpenSquareBracket = true;
                            chClass = 0 /* None */;
                        }
                        else {
                            chClass = classifier.get(chCode);
                        }
                        // Check if character terminates link
                        if (chClass === 1 /* ForceTermination */) {
                            resetStateMachine = true;
                        }
                        else {
                            state = 13 /* Accept */;
                        }
                    }
                    else {
                        state = stateMachine.nextState(state, chCode);
                        if (state === 0 /* Invalid */) {
                            resetStateMachine = true;
                        }
                    }
                    if (resetStateMachine) {
                        state = 1 /* Start */;
                        hasOpenParens = false;
                        hasOpenSquareBracket = false;
                        hasOpenCurlyBracket = false;
                        // Record where the link started
                        linkBeginIndex = j + 1;
                        linkBeginChCode = chCode;
                    }
                    j++;
                }
                if (state === 13 /* Accept */) {
                    result.push(LinkComputer._createLink(classifier, line, i, linkBeginIndex, len));
                }
            }
            return result;
        };
        return LinkComputer;
    }());
    exports.LinkComputer = LinkComputer;
    /**
     * Returns an array of all links contains in the provided
     * document. *Note* that this operation is computational
     * expensive and should not run in the UI thread.
     */
    function computeLinks(model) {
        if (!model || typeof model.getLineCount !== 'function' || typeof model.getLineContent !== 'function') {
            // Unknown caller!
            return [];
        }
        return LinkComputer.computeLinks(model);
    }
    exports.computeLinks = computeLinks;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[28/*vs/editor/common/modes/supports/inplaceReplaceSupport*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var BasicInplaceReplace = /** @class */ (function () {
        function BasicInplaceReplace() {
            this._defaultValueSet = [
                ['true', 'false'],
                ['True', 'False'],
                ['Private', 'Public', 'Friend', 'ReadOnly', 'Partial', 'Protected', 'WriteOnly'],
                ['public', 'protected', 'private'],
            ];
        }
        BasicInplaceReplace.prototype.navigateValueSet = function (range1, text1, range2, text2, up) {
            if (range1 && text1) {
                var result = this.doNavigateValueSet(text1, up);
                if (result) {
                    return {
                        range: range1,
                        value: result
                    };
                }
            }
            if (range2 && text2) {
                var result = this.doNavigateValueSet(text2, up);
                if (result) {
                    return {
                        range: range2,
                        value: result
                    };
                }
            }
            return null;
        };
        BasicInplaceReplace.prototype.doNavigateValueSet = function (text, up) {
            var numberResult = this.numberReplace(text, up);
            if (numberResult !== null) {
                return numberResult;
            }
            return this.textReplace(text, up);
        };
        BasicInplaceReplace.prototype.numberReplace = function (value, up) {
            var precision = Math.pow(10, value.length - (value.lastIndexOf('.') + 1));
            var n1 = Number(value);
            var n2 = parseFloat(value);
            if (!isNaN(n1) && !isNaN(n2) && n1 === n2) {
                if (n1 === 0 && !up) {
                    return null; // don't do negative
                    //			} else if(n1 === 9 && up) {
                    //				return null; // don't insert 10 into a number
                }
                else {
                    n1 = Math.floor(n1 * precision);
                    n1 += up ? precision : -precision;
                    return String(n1 / precision);
                }
            }
            return null;
        };
        BasicInplaceReplace.prototype.textReplace = function (value, up) {
            return this.valueSetsReplace(this._defaultValueSet, value, up);
        };
        BasicInplaceReplace.prototype.valueSetsReplace = function (valueSets, value, up) {
            var result = null;
            for (var i = 0, len = valueSets.length; result === null && i < len; i++) {
                result = this.valueSetReplace(valueSets[i], value, up);
            }
            return result;
        };
        BasicInplaceReplace.prototype.valueSetReplace = function (valueSet, value, up) {
            var idx = valueSet.indexOf(value);
            if (idx >= 0) {
                idx += up ? +1 : -1;
                if (idx < 0) {
                    idx = valueSet.length - 1;
                }
                else {
                    idx %= valueSet.length;
                }
                return valueSet[idx];
            }
            return null;
        };
        BasicInplaceReplace.INSTANCE = new BasicInplaceReplace();
        return BasicInplaceReplace;
    }());
    exports.BasicInplaceReplace = BasicInplaceReplace;
});

/*!
Copyright (c) 2014 Taylor Hakes
Copyright (c) 2014 Forbes Lindesay
 */
(function (global, factory) {
	typeof exports === 'object' && typeof module !== 'undefined' ? factory() :
		typeof define === 'function' && define.amd ? define("vs/editor/common/standalone/promise-polyfill/polyfill", factory) :
			(factory());
}(this, (function () {
	'use strict';

	/**
	 * @this {Promise}
	 */
	function finallyConstructor(callback) {
		var constructor = this.constructor;
		return this.then(
			function (value) {
				return constructor.resolve(callback()).then(function () {
					return value;
				});
			},
			function (reason) {
				return constructor.resolve(callback()).then(function () {
					return constructor.reject(reason);
				});
			}
		);
	}

	// Store setTimeout reference so promise-polyfill will be unaffected by
	// other code modifying setTimeout (like sinon.useFakeTimers())
	var setTimeoutFunc = setTimeout;

	function noop() { }

	// Polyfill for Function.prototype.bind
	function bind(fn, thisArg) {
		return function () {
			fn.apply(thisArg, arguments);
		};
	}

	/**
	 * @constructor
	 * @param {Function} fn
	 */
	function Promise(fn) {
		if (!(this instanceof Promise))
			throw new TypeError('Promises must be constructed via new');
		if (typeof fn !== 'function') throw new TypeError('not a function');
		/** @type {!number} */
		this._state = 0;
		/** @type {!boolean} */
		this._handled = false;
		/** @type {Promise|undefined} */
		this._value = undefined;
		/** @type {!Array<!Function>} */
		this._deferreds = [];

		doResolve(fn, this);
	}

	function handle(self, deferred) {
		while (self._state === 3) {
			self = self._value;
		}
		if (self._state === 0) {
			self._deferreds.push(deferred);
			return;
		}
		self._handled = true;
		Promise._immediateFn(function () {
			var cb = self._state === 1 ? deferred.onFulfilled : deferred.onRejected;
			if (cb === null) {
				(self._state === 1 ? resolve : reject)(deferred.promise, self._value);
				return;
			}
			var ret;
			try {
				ret = cb(self._value);
			} catch (e) {
				reject(deferred.promise, e);
				return;
			}
			resolve(deferred.promise, ret);
		});
	}

	function resolve(self, newValue) {
		try {
			// Promise Resolution Procedure: https://github.com/promises-aplus/promises-spec#the-promise-resolution-procedure
			if (newValue === self)
				throw new TypeError('A promise cannot be resolved with itself.');
			if (
				newValue &&
				(typeof newValue === 'object' || typeof newValue === 'function')
			) {
				var then = newValue.then;
				if (newValue instanceof Promise) {
					self._state = 3;
					self._value = newValue;
					finale(self);
					return;
				} else if (typeof then === 'function') {
					doResolve(bind(then, newValue), self);
					return;
				}
			}
			self._state = 1;
			self._value = newValue;
			finale(self);
		} catch (e) {
			reject(self, e);
		}
	}

	function reject(self, newValue) {
		self._state = 2;
		self._value = newValue;
		finale(self);
	}

	function finale(self) {
		if (self._state === 2 && self._deferreds.length === 0) {
			Promise._immediateFn(function () {
				if (!self._handled) {
					Promise._unhandledRejectionFn(self._value);
				}
			});
		}

		for (var i = 0, len = self._deferreds.length; i < len; i++) {
			handle(self, self._deferreds[i]);
		}
		self._deferreds = null;
	}

	/**
	 * @constructor
	 */
	function Handler(onFulfilled, onRejected, promise) {
		this.onFulfilled = typeof onFulfilled === 'function' ? onFulfilled : null;
		this.onRejected = typeof onRejected === 'function' ? onRejected : null;
		this.promise = promise;
	}

	/**
	 * Take a potentially misbehaving resolver function and make sure
	 * onFulfilled and onRejected are only called once.
	 *
	 * Makes no guarantees about asynchrony.
	 */
	function doResolve(fn, self) {
		var done = false;
		try {
			fn(
				function (value) {
					if (done) return;
					done = true;
					resolve(self, value);
				},
				function (reason) {
					if (done) return;
					done = true;
					reject(self, reason);
				}
			);
		} catch (ex) {
			if (done) return;
			done = true;
			reject(self, ex);
		}
	}

	Promise.prototype['catch'] = function (onRejected) {
		return this.then(null, onRejected);
	};

	Promise.prototype.then = function (onFulfilled, onRejected) {
		// @ts-ignore
		var prom = new this.constructor(noop);

		handle(this, new Handler(onFulfilled, onRejected, prom));
		return prom;
	};

	Promise.prototype['finally'] = finallyConstructor;

	Promise.all = function (arr) {
		return new Promise(function (resolve, reject) {
			if (!arr || typeof arr.length === 'undefined')
				throw new TypeError('Promise.all accepts an array');
			var args = Array.prototype.slice.call(arr);
			if (args.length === 0) return resolve([]);
			var remaining = args.length;

			function res(i, val) {
				try {
					if (val && (typeof val === 'object' || typeof val === 'function')) {
						var then = val.then;
						if (typeof then === 'function') {
							then.call(
								val,
								function (val) {
									res(i, val);
								},
								reject
							);
							return;
						}
					}
					args[i] = val;
					if (--remaining === 0) {
						resolve(args);
					}
				} catch (ex) {
					reject(ex);
				}
			}

			for (var i = 0; i < args.length; i++) {
				res(i, args[i]);
			}
		});
	};

	Promise.resolve = function (value) {
		if (value && typeof value === 'object' && value.constructor === Promise) {
			return value;
		}

		return new Promise(function (resolve) {
			resolve(value);
		});
	};

	Promise.reject = function (value) {
		return new Promise(function (resolve, reject) {
			reject(value);
		});
	};

	Promise.race = function (values) {
		return new Promise(function (resolve, reject) {
			for (var i = 0, len = values.length; i < len; i++) {
				values[i].then(resolve, reject);
			}
		});
	};

	// Use polyfill for setImmediate for performance gains
	Promise._immediateFn =
		(typeof setImmediate === 'function' &&
			function (fn) {
				setImmediate(fn);
			}) ||
		function (fn) {
			setTimeoutFunc(fn, 0);
		};

	Promise._unhandledRejectionFn = function _unhandledRejectionFn(err) {
		if (typeof console !== 'undefined' && console) {
			console.warn('Possible Unhandled Promise Rejection:', err); // eslint-disable-line no-console
		}
	};

	/** @suppress {undefinedVars} */
	var globalNS = (function () {
		// the only reliable means to get the global object is
		// `Function('return this')()`
		// However, this causes CSP violations in Chrome apps.
		if (typeof self !== 'undefined') {
			return self;
		}
		if (typeof window !== 'undefined') {
			return window;
		}
		if (typeof global !== 'undefined') {
			return global;
		}
		throw new Error('unable to locate global object');
	})();

	if (!('Promise' in globalNS)) {
		globalNS['Promise'] = Promise;
	} else if (!globalNS.Promise.prototype['finally']) {
		globalNS.Promise.prototype['finally'] = finallyConstructor;
	}

})));

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[29/*vs/editor/common/standalone/standaloneEnums*/], __M([1/*require*/,0/*exports*/]), function (require, exports) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    // THIS IS A GENERATED FILE. DO NOT EDIT DIRECTLY.
    var AccessibilitySupport;
    (function (AccessibilitySupport) {
        /**
         * This should be the browser case where it is not known if a screen reader is attached or no.
         */
        AccessibilitySupport[AccessibilitySupport["Unknown"] = 0] = "Unknown";
        AccessibilitySupport[AccessibilitySupport["Disabled"] = 1] = "Disabled";
        AccessibilitySupport[AccessibilitySupport["Enabled"] = 2] = "Enabled";
    })(AccessibilitySupport = exports.AccessibilitySupport || (exports.AccessibilitySupport = {}));
    var CompletionItemInsertTextRule;
    (function (CompletionItemInsertTextRule) {
        /**
         * Adjust whitespace/indentation of multiline insert texts to
         * match the current line indentation.
         */
        CompletionItemInsertTextRule[CompletionItemInsertTextRule["KeepWhitespace"] = 1] = "KeepWhitespace";
        /**
         * `insertText` is a snippet.
         */
        CompletionItemInsertTextRule[CompletionItemInsertTextRule["InsertAsSnippet"] = 4] = "InsertAsSnippet";
    })(CompletionItemInsertTextRule = exports.CompletionItemInsertTextRule || (exports.CompletionItemInsertTextRule = {}));
    var CompletionItemKind;
    (function (CompletionItemKind) {
        CompletionItemKind[CompletionItemKind["Method"] = 0] = "Method";
        CompletionItemKind[CompletionItemKind["Function"] = 1] = "Function";
        CompletionItemKind[CompletionItemKind["Constructor"] = 2] = "Constructor";
        CompletionItemKind[CompletionItemKind["Field"] = 3] = "Field";
        CompletionItemKind[CompletionItemKind["Variable"] = 4] = "Variable";
        CompletionItemKind[CompletionItemKind["Class"] = 5] = "Class";
        CompletionItemKind[CompletionItemKind["Struct"] = 6] = "Struct";
        CompletionItemKind[CompletionItemKind["Interface"] = 7] = "Interface";
        CompletionItemKind[CompletionItemKind["Module"] = 8] = "Module";
        CompletionItemKind[CompletionItemKind["Property"] = 9] = "Property";
        CompletionItemKind[CompletionItemKind["Event"] = 10] = "Event";
        CompletionItemKind[CompletionItemKind["Operator"] = 11] = "Operator";
        CompletionItemKind[CompletionItemKind["Unit"] = 12] = "Unit";
        CompletionItemKind[CompletionItemKind["Value"] = 13] = "Value";
        CompletionItemKind[CompletionItemKind["Constant"] = 14] = "Constant";
        CompletionItemKind[CompletionItemKind["Enum"] = 15] = "Enum";
        CompletionItemKind[CompletionItemKind["EnumMember"] = 16] = "EnumMember";
        CompletionItemKind[CompletionItemKind["Keyword"] = 17] = "Keyword";
        CompletionItemKind[CompletionItemKind["Text"] = 18] = "Text";
        CompletionItemKind[CompletionItemKind["Color"] = 19] = "Color";
        CompletionItemKind[CompletionItemKind["File"] = 20] = "File";
        CompletionItemKind[CompletionItemKind["Reference"] = 21] = "Reference";
        CompletionItemKind[CompletionItemKind["Customcolor"] = 22] = "Customcolor";
        CompletionItemKind[CompletionItemKind["Folder"] = 23] = "Folder";
        CompletionItemKind[CompletionItemKind["TypeParameter"] = 24] = "TypeParameter";
        CompletionItemKind[CompletionItemKind["Snippet"] = 25] = "Snippet";
    })(CompletionItemKind = exports.CompletionItemKind || (exports.CompletionItemKind = {}));
    var CompletionItemTag;
    (function (CompletionItemTag) {
        CompletionItemTag[CompletionItemTag["Deprecated"] = 1] = "Deprecated";
    })(CompletionItemTag = exports.CompletionItemTag || (exports.CompletionItemTag = {}));
    /**
     * How a suggest provider was triggered.
     */
    var CompletionTriggerKind;
    (function (CompletionTriggerKind) {
        CompletionTriggerKind[CompletionTriggerKind["Invoke"] = 0] = "Invoke";
        CompletionTriggerKind[CompletionTriggerKind["TriggerCharacter"] = 1] = "TriggerCharacter";
        CompletionTriggerKind[CompletionTriggerKind["TriggerForIncompleteCompletions"] = 2] = "TriggerForIncompleteCompletions";
    })(CompletionTriggerKind = exports.CompletionTriggerKind || (exports.CompletionTriggerKind = {}));
    /**
     * A positioning preference for rendering content widgets.
     */
    var ContentWidgetPositionPreference;
    (function (ContentWidgetPositionPreference) {
        /**
         * Place the content widget exactly at a position
         */
        ContentWidgetPositionPreference[ContentWidgetPositionPreference["EXACT"] = 0] = "EXACT";
        /**
         * Place the content widget above a position
         */
        ContentWidgetPositionPreference[ContentWidgetPositionPreference["ABOVE"] = 1] = "ABOVE";
        /**
         * Place the content widget below a position
         */
        ContentWidgetPositionPreference[ContentWidgetPositionPreference["BELOW"] = 2] = "BELOW";
    })(ContentWidgetPositionPreference = exports.ContentWidgetPositionPreference || (exports.ContentWidgetPositionPreference = {}));
    /**
     * Describes the reason the cursor has changed its position.
     */
    var CursorChangeReason;
    (function (CursorChangeReason) {
        /**
         * Unknown or not set.
         */
        CursorChangeReason[CursorChangeReason["NotSet"] = 0] = "NotSet";
        /**
         * A `model.setValue()` was called.
         */
        CursorChangeReason[CursorChangeReason["ContentFlush"] = 1] = "ContentFlush";
        /**
         * The `model` has been changed outside of this cursor and the cursor recovers its position from associated markers.
         */
        CursorChangeReason[CursorChangeReason["RecoverFromMarkers"] = 2] = "RecoverFromMarkers";
        /**
         * There was an explicit user gesture.
         */
        CursorChangeReason[CursorChangeReason["Explicit"] = 3] = "Explicit";
        /**
         * There was a Paste.
         */
        CursorChangeReason[CursorChangeReason["Paste"] = 4] = "Paste";
        /**
         * There was an Undo.
         */
        CursorChangeReason[CursorChangeReason["Undo"] = 5] = "Undo";
        /**
         * There was a Redo.
         */
        CursorChangeReason[CursorChangeReason["Redo"] = 6] = "Redo";
    })(CursorChangeReason = exports.CursorChangeReason || (exports.CursorChangeReason = {}));
    /**
     * The default end of line to use when instantiating models.
     */
    var DefaultEndOfLine;
    (function (DefaultEndOfLine) {
        /**
         * Use line feed (\n) as the end of line character.
         */
        DefaultEndOfLine[DefaultEndOfLine["LF"] = 1] = "LF";
        /**
         * Use carriage return and line feed (\r\n) as the end of line character.
         */
        DefaultEndOfLine[DefaultEndOfLine["CRLF"] = 2] = "CRLF";
    })(DefaultEndOfLine = exports.DefaultEndOfLine || (exports.DefaultEndOfLine = {}));
    /**
     * A document highlight kind.
     */
    var DocumentHighlightKind;
    (function (DocumentHighlightKind) {
        /**
         * A textual occurrence.
         */
        DocumentHighlightKind[DocumentHighlightKind["Text"] = 0] = "Text";
        /**
         * Read-access of a symbol, like reading a variable.
         */
        DocumentHighlightKind[DocumentHighlightKind["Read"] = 1] = "Read";
        /**
         * Write-access of a symbol, like writing to a variable.
         */
        DocumentHighlightKind[DocumentHighlightKind["Write"] = 2] = "Write";
    })(DocumentHighlightKind = exports.DocumentHighlightKind || (exports.DocumentHighlightKind = {}));
    /**
     * Configuration options for auto indentation in the editor
     */
    var EditorAutoIndentStrategy;
    (function (EditorAutoIndentStrategy) {
        EditorAutoIndentStrategy[EditorAutoIndentStrategy["None"] = 0] = "None";
        EditorAutoIndentStrategy[EditorAutoIndentStrategy["Keep"] = 1] = "Keep";
        EditorAutoIndentStrategy[EditorAutoIndentStrategy["Brackets"] = 2] = "Brackets";
        EditorAutoIndentStrategy[EditorAutoIndentStrategy["Advanced"] = 3] = "Advanced";
        EditorAutoIndentStrategy[EditorAutoIndentStrategy["Full"] = 4] = "Full";
    })(EditorAutoIndentStrategy = exports.EditorAutoIndentStrategy || (exports.EditorAutoIndentStrategy = {}));
    var EditorOption;
    (function (EditorOption) {
        EditorOption[EditorOption["acceptSuggestionOnCommitCharacter"] = 0] = "acceptSuggestionOnCommitCharacter";
        EditorOption[EditorOption["acceptSuggestionOnEnter"] = 1] = "acceptSuggestionOnEnter";
        EditorOption[EditorOption["accessibilitySupport"] = 2] = "accessibilitySupport";
        EditorOption[EditorOption["accessibilityPageSize"] = 3] = "accessibilityPageSize";
        EditorOption[EditorOption["ariaLabel"] = 4] = "ariaLabel";
        EditorOption[EditorOption["autoClosingBrackets"] = 5] = "autoClosingBrackets";
        EditorOption[EditorOption["autoClosingOvertype"] = 6] = "autoClosingOvertype";
        EditorOption[EditorOption["autoClosingQuotes"] = 7] = "autoClosingQuotes";
        EditorOption[EditorOption["autoIndent"] = 8] = "autoIndent";
        EditorOption[EditorOption["automaticLayout"] = 9] = "automaticLayout";
        EditorOption[EditorOption["autoSurround"] = 10] = "autoSurround";
        EditorOption[EditorOption["codeLens"] = 11] = "codeLens";
        EditorOption[EditorOption["colorDecorators"] = 12] = "colorDecorators";
        EditorOption[EditorOption["comments"] = 13] = "comments";
        EditorOption[EditorOption["contextmenu"] = 14] = "contextmenu";
        EditorOption[EditorOption["copyWithSyntaxHighlighting"] = 15] = "copyWithSyntaxHighlighting";
        EditorOption[EditorOption["cursorBlinking"] = 16] = "cursorBlinking";
        EditorOption[EditorOption["cursorSmoothCaretAnimation"] = 17] = "cursorSmoothCaretAnimation";
        EditorOption[EditorOption["cursorStyle"] = 18] = "cursorStyle";
        EditorOption[EditorOption["cursorSurroundingLines"] = 19] = "cursorSurroundingLines";
        EditorOption[EditorOption["cursorSurroundingLinesStyle"] = 20] = "cursorSurroundingLinesStyle";
        EditorOption[EditorOption["cursorWidth"] = 21] = "cursorWidth";
        EditorOption[EditorOption["disableLayerHinting"] = 22] = "disableLayerHinting";
        EditorOption[EditorOption["disableMonospaceOptimizations"] = 23] = "disableMonospaceOptimizations";
        EditorOption[EditorOption["dragAndDrop"] = 24] = "dragAndDrop";
        EditorOption[EditorOption["emptySelectionClipboard"] = 25] = "emptySelectionClipboard";
        EditorOption[EditorOption["extraEditorClassName"] = 26] = "extraEditorClassName";
        EditorOption[EditorOption["fastScrollSensitivity"] = 27] = "fastScrollSensitivity";
        EditorOption[EditorOption["find"] = 28] = "find";
        EditorOption[EditorOption["fixedOverflowWidgets"] = 29] = "fixedOverflowWidgets";
        EditorOption[EditorOption["folding"] = 30] = "folding";
        EditorOption[EditorOption["foldingStrategy"] = 31] = "foldingStrategy";
        EditorOption[EditorOption["foldingHighlight"] = 32] = "foldingHighlight";
        EditorOption[EditorOption["fontFamily"] = 33] = "fontFamily";
        EditorOption[EditorOption["fontInfo"] = 34] = "fontInfo";
        EditorOption[EditorOption["fontLigatures"] = 35] = "fontLigatures";
        EditorOption[EditorOption["fontSize"] = 36] = "fontSize";
        EditorOption[EditorOption["fontWeight"] = 37] = "fontWeight";
        EditorOption[EditorOption["formatOnPaste"] = 38] = "formatOnPaste";
        EditorOption[EditorOption["formatOnType"] = 39] = "formatOnType";
        EditorOption[EditorOption["glyphMargin"] = 40] = "glyphMargin";
        EditorOption[EditorOption["gotoLocation"] = 41] = "gotoLocation";
        EditorOption[EditorOption["hideCursorInOverviewRuler"] = 42] = "hideCursorInOverviewRuler";
        EditorOption[EditorOption["highlightActiveIndentGuide"] = 43] = "highlightActiveIndentGuide";
        EditorOption[EditorOption["hover"] = 44] = "hover";
        EditorOption[EditorOption["inDiffEditor"] = 45] = "inDiffEditor";
        EditorOption[EditorOption["letterSpacing"] = 46] = "letterSpacing";
        EditorOption[EditorOption["lightbulb"] = 47] = "lightbulb";
        EditorOption[EditorOption["lineDecorationsWidth"] = 48] = "lineDecorationsWidth";
        EditorOption[EditorOption["lineHeight"] = 49] = "lineHeight";
        EditorOption[EditorOption["lineNumbers"] = 50] = "lineNumbers";
        EditorOption[EditorOption["lineNumbersMinChars"] = 51] = "lineNumbersMinChars";
        EditorOption[EditorOption["links"] = 52] = "links";
        EditorOption[EditorOption["matchBrackets"] = 53] = "matchBrackets";
        EditorOption[EditorOption["minimap"] = 54] = "minimap";
        EditorOption[EditorOption["mouseStyle"] = 55] = "mouseStyle";
        EditorOption[EditorOption["mouseWheelScrollSensitivity"] = 56] = "mouseWheelScrollSensitivity";
        EditorOption[EditorOption["mouseWheelZoom"] = 57] = "mouseWheelZoom";
        EditorOption[EditorOption["multiCursorMergeOverlapping"] = 58] = "multiCursorMergeOverlapping";
        EditorOption[EditorOption["multiCursorModifier"] = 59] = "multiCursorModifier";
        EditorOption[EditorOption["multiCursorPaste"] = 60] = "multiCursorPaste";
        EditorOption[EditorOption["occurrencesHighlight"] = 61] = "occurrencesHighlight";
        EditorOption[EditorOption["overviewRulerBorder"] = 62] = "overviewRulerBorder";
        EditorOption[EditorOption["overviewRulerLanes"] = 63] = "overviewRulerLanes";
        EditorOption[EditorOption["parameterHints"] = 64] = "parameterHints";
        EditorOption[EditorOption["peekWidgetDefaultFocus"] = 65] = "peekWidgetDefaultFocus";
        EditorOption[EditorOption["quickSuggestions"] = 66] = "quickSuggestions";
        EditorOption[EditorOption["quickSuggestionsDelay"] = 67] = "quickSuggestionsDelay";
        EditorOption[EditorOption["readOnly"] = 68] = "readOnly";
        EditorOption[EditorOption["renderControlCharacters"] = 69] = "renderControlCharacters";
        EditorOption[EditorOption["renderIndentGuides"] = 70] = "renderIndentGuides";
        EditorOption[EditorOption["renderFinalNewline"] = 71] = "renderFinalNewline";
        EditorOption[EditorOption["renderLineHighlight"] = 72] = "renderLineHighlight";
        EditorOption[EditorOption["renderValidationDecorations"] = 73] = "renderValidationDecorations";
        EditorOption[EditorOption["renderWhitespace"] = 74] = "renderWhitespace";
        EditorOption[EditorOption["revealHorizontalRightPadding"] = 75] = "revealHorizontalRightPadding";
        EditorOption[EditorOption["roundedSelection"] = 76] = "roundedSelection";
        EditorOption[EditorOption["rulers"] = 77] = "rulers";
        EditorOption[EditorOption["scrollbar"] = 78] = "scrollbar";
        EditorOption[EditorOption["scrollBeyondLastColumn"] = 79] = "scrollBeyondLastColumn";
        EditorOption[EditorOption["scrollBeyondLastLine"] = 80] = "scrollBeyondLastLine";
        EditorOption[EditorOption["selectionClipboard"] = 81] = "selectionClipboard";
        EditorOption[EditorOption["selectionHighlight"] = 82] = "selectionHighlight";
        EditorOption[EditorOption["selectOnLineNumbers"] = 83] = "selectOnLineNumbers";
        EditorOption[EditorOption["showFoldingControls"] = 84] = "showFoldingControls";
        EditorOption[EditorOption["showUnused"] = 85] = "showUnused";
        EditorOption[EditorOption["snippetSuggestions"] = 86] = "snippetSuggestions";
        EditorOption[EditorOption["smoothScrolling"] = 87] = "smoothScrolling";
        EditorOption[EditorOption["stopRenderingLineAfter"] = 88] = "stopRenderingLineAfter";
        EditorOption[EditorOption["suggest"] = 89] = "suggest";
        EditorOption[EditorOption["suggestFontSize"] = 90] = "suggestFontSize";
        EditorOption[EditorOption["suggestLineHeight"] = 91] = "suggestLineHeight";
        EditorOption[EditorOption["suggestOnTriggerCharacters"] = 92] = "suggestOnTriggerCharacters";
        EditorOption[EditorOption["suggestSelection"] = 93] = "suggestSelection";
        EditorOption[EditorOption["tabCompletion"] = 94] = "tabCompletion";
        EditorOption[EditorOption["useTabStops"] = 95] = "useTabStops";
        EditorOption[EditorOption["wordSeparators"] = 96] = "wordSeparators";
        EditorOption[EditorOption["wordWrap"] = 97] = "wordWrap";
        EditorOption[EditorOption["wordWrapBreakAfterCharacters"] = 98] = "wordWrapBreakAfterCharacters";
        EditorOption[EditorOption["wordWrapBreakBeforeCharacters"] = 99] = "wordWrapBreakBeforeCharacters";
        EditorOption[EditorOption["wordWrapColumn"] = 100] = "wordWrapColumn";
        EditorOption[EditorOption["wordWrapMinified"] = 101] = "wordWrapMinified";
        EditorOption[EditorOption["wrappingIndent"] = 102] = "wrappingIndent";
        EditorOption[EditorOption["wrappingStrategy"] = 103] = "wrappingStrategy";
        EditorOption[EditorOption["editorClassName"] = 104] = "editorClassName";
        EditorOption[EditorOption["pixelRatio"] = 105] = "pixelRatio";
        EditorOption[EditorOption["tabFocusMode"] = 106] = "tabFocusMode";
        EditorOption[EditorOption["layoutInfo"] = 107] = "layoutInfo";
        EditorOption[EditorOption["wrappingInfo"] = 108] = "wrappingInfo";
    })(EditorOption = exports.EditorOption || (exports.EditorOption = {}));
    /**
     * End of line character preference.
     */
    var EndOfLinePreference;
    (function (EndOfLinePreference) {
        /**
         * Use the end of line character identified in the text buffer.
         */
        EndOfLinePreference[EndOfLinePreference["TextDefined"] = 0] = "TextDefined";
        /**
         * Use line feed (\n) as the end of line character.
         */
        EndOfLinePreference[EndOfLinePreference["LF"] = 1] = "LF";
        /**
         * Use carriage return and line feed (\r\n) as the end of line character.
         */
        EndOfLinePreference[EndOfLinePreference["CRLF"] = 2] = "CRLF";
    })(EndOfLinePreference = exports.EndOfLinePreference || (exports.EndOfLinePreference = {}));
    /**
     * End of line character preference.
     */
    var EndOfLineSequence;
    (function (EndOfLineSequence) {
        /**
         * Use line feed (\n) as the end of line character.
         */
        EndOfLineSequence[EndOfLineSequence["LF"] = 0] = "LF";
        /**
         * Use carriage return and line feed (\r\n) as the end of line character.
         */
        EndOfLineSequence[EndOfLineSequence["CRLF"] = 1] = "CRLF";
    })(EndOfLineSequence = exports.EndOfLineSequence || (exports.EndOfLineSequence = {}));
    /**
     * Describes what to do with the indentation when pressing Enter.
     */
    var IndentAction;
    (function (IndentAction) {
        /**
         * Insert new line and copy the previous line's indentation.
         */
        IndentAction[IndentAction["None"] = 0] = "None";
        /**
         * Insert new line and indent once (relative to the previous line's indentation).
         */
        IndentAction[IndentAction["Indent"] = 1] = "Indent";
        /**
         * Insert two new lines:
         *  - the first one indented which will hold the cursor
         *  - the second one at the same indentation level
         */
        IndentAction[IndentAction["IndentOutdent"] = 2] = "IndentOutdent";
        /**
         * Insert new line and outdent once (relative to the previous line's indentation).
         */
        IndentAction[IndentAction["Outdent"] = 3] = "Outdent";
    })(IndentAction = exports.IndentAction || (exports.IndentAction = {}));
    /**
     * Virtual Key Codes, the value does not hold any inherent meaning.
     * Inspired somewhat from https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
     * But these are "more general", as they should work across browsers & OS`s.
     */
    var KeyCode;
    (function (KeyCode) {
        /**
         * Placed first to cover the 0 value of the enum.
         */
        KeyCode[KeyCode["Unknown"] = 0] = "Unknown";
        KeyCode[KeyCode["Backspace"] = 1] = "Backspace";
        KeyCode[KeyCode["Tab"] = 2] = "Tab";
        KeyCode[KeyCode["Enter"] = 3] = "Enter";
        KeyCode[KeyCode["Shift"] = 4] = "Shift";
        KeyCode[KeyCode["Ctrl"] = 5] = "Ctrl";
        KeyCode[KeyCode["Alt"] = 6] = "Alt";
        KeyCode[KeyCode["PauseBreak"] = 7] = "PauseBreak";
        KeyCode[KeyCode["CapsLock"] = 8] = "CapsLock";
        KeyCode[KeyCode["Escape"] = 9] = "Escape";
        KeyCode[KeyCode["Space"] = 10] = "Space";
        KeyCode[KeyCode["PageUp"] = 11] = "PageUp";
        KeyCode[KeyCode["PageDown"] = 12] = "PageDown";
        KeyCode[KeyCode["End"] = 13] = "End";
        KeyCode[KeyCode["Home"] = 14] = "Home";
        KeyCode[KeyCode["LeftArrow"] = 15] = "LeftArrow";
        KeyCode[KeyCode["UpArrow"] = 16] = "UpArrow";
        KeyCode[KeyCode["RightArrow"] = 17] = "RightArrow";
        KeyCode[KeyCode["DownArrow"] = 18] = "DownArrow";
        KeyCode[KeyCode["Insert"] = 19] = "Insert";
        KeyCode[KeyCode["Delete"] = 20] = "Delete";
        KeyCode[KeyCode["KEY_0"] = 21] = "KEY_0";
        KeyCode[KeyCode["KEY_1"] = 22] = "KEY_1";
        KeyCode[KeyCode["KEY_2"] = 23] = "KEY_2";
        KeyCode[KeyCode["KEY_3"] = 24] = "KEY_3";
        KeyCode[KeyCode["KEY_4"] = 25] = "KEY_4";
        KeyCode[KeyCode["KEY_5"] = 26] = "KEY_5";
        KeyCode[KeyCode["KEY_6"] = 27] = "KEY_6";
        KeyCode[KeyCode["KEY_7"] = 28] = "KEY_7";
        KeyCode[KeyCode["KEY_8"] = 29] = "KEY_8";
        KeyCode[KeyCode["KEY_9"] = 30] = "KEY_9";
        KeyCode[KeyCode["KEY_A"] = 31] = "KEY_A";
        KeyCode[KeyCode["KEY_B"] = 32] = "KEY_B";
        KeyCode[KeyCode["KEY_C"] = 33] = "KEY_C";
        KeyCode[KeyCode["KEY_D"] = 34] = "KEY_D";
        KeyCode[KeyCode["KEY_E"] = 35] = "KEY_E";
        KeyCode[KeyCode["KEY_F"] = 36] = "KEY_F";
        KeyCode[KeyCode["KEY_G"] = 37] = "KEY_G";
        KeyCode[KeyCode["KEY_H"] = 38] = "KEY_H";
        KeyCode[KeyCode["KEY_I"] = 39] = "KEY_I";
        KeyCode[KeyCode["KEY_J"] = 40] = "KEY_J";
        KeyCode[KeyCode["KEY_K"] = 41] = "KEY_K";
        KeyCode[KeyCode["KEY_L"] = 42] = "KEY_L";
        KeyCode[KeyCode["KEY_M"] = 43] = "KEY_M";
        KeyCode[KeyCode["KEY_N"] = 44] = "KEY_N";
        KeyCode[KeyCode["KEY_O"] = 45] = "KEY_O";
        KeyCode[KeyCode["KEY_P"] = 46] = "KEY_P";
        KeyCode[KeyCode["KEY_Q"] = 47] = "KEY_Q";
        KeyCode[KeyCode["KEY_R"] = 48] = "KEY_R";
        KeyCode[KeyCode["KEY_S"] = 49] = "KEY_S";
        KeyCode[KeyCode["KEY_T"] = 50] = "KEY_T";
        KeyCode[KeyCode["KEY_U"] = 51] = "KEY_U";
        KeyCode[KeyCode["KEY_V"] = 52] = "KEY_V";
        KeyCode[KeyCode["KEY_W"] = 53] = "KEY_W";
        KeyCode[KeyCode["KEY_X"] = 54] = "KEY_X";
        KeyCode[KeyCode["KEY_Y"] = 55] = "KEY_Y";
        KeyCode[KeyCode["KEY_Z"] = 56] = "KEY_Z";
        KeyCode[KeyCode["Meta"] = 57] = "Meta";
        KeyCode[KeyCode["ContextMenu"] = 58] = "ContextMenu";
        KeyCode[KeyCode["F1"] = 59] = "F1";
        KeyCode[KeyCode["F2"] = 60] = "F2";
        KeyCode[KeyCode["F3"] = 61] = "F3";
        KeyCode[KeyCode["F4"] = 62] = "F4";
        KeyCode[KeyCode["F5"] = 63] = "F5";
        KeyCode[KeyCode["F6"] = 64] = "F6";
        KeyCode[KeyCode["F7"] = 65] = "F7";
        KeyCode[KeyCode["F8"] = 66] = "F8";
        KeyCode[KeyCode["F9"] = 67] = "F9";
        KeyCode[KeyCode["F10"] = 68] = "F10";
        KeyCode[KeyCode["F11"] = 69] = "F11";
        KeyCode[KeyCode["F12"] = 70] = "F12";
        KeyCode[KeyCode["F13"] = 71] = "F13";
        KeyCode[KeyCode["F14"] = 72] = "F14";
        KeyCode[KeyCode["F15"] = 73] = "F15";
        KeyCode[KeyCode["F16"] = 74] = "F16";
        KeyCode[KeyCode["F17"] = 75] = "F17";
        KeyCode[KeyCode["F18"] = 76] = "F18";
        KeyCode[KeyCode["F19"] = 77] = "F19";
        KeyCode[KeyCode["NumLock"] = 78] = "NumLock";
        KeyCode[KeyCode["ScrollLock"] = 79] = "ScrollLock";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the ';:' key
         */
        KeyCode[KeyCode["US_SEMICOLON"] = 80] = "US_SEMICOLON";
        /**
         * For any country/region, the '+' key
         * For the US standard keyboard, the '=+' key
         */
        KeyCode[KeyCode["US_EQUAL"] = 81] = "US_EQUAL";
        /**
         * For any country/region, the ',' key
         * For the US standard keyboard, the ',<' key
         */
        KeyCode[KeyCode["US_COMMA"] = 82] = "US_COMMA";
        /**
         * For any country/region, the '-' key
         * For the US standard keyboard, the '-_' key
         */
        KeyCode[KeyCode["US_MINUS"] = 83] = "US_MINUS";
        /**
         * For any country/region, the '.' key
         * For the US standard keyboard, the '.>' key
         */
        KeyCode[KeyCode["US_DOT"] = 84] = "US_DOT";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the '/?' key
         */
        KeyCode[KeyCode["US_SLASH"] = 85] = "US_SLASH";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the '`~' key
         */
        KeyCode[KeyCode["US_BACKTICK"] = 86] = "US_BACKTICK";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the '[{' key
         */
        KeyCode[KeyCode["US_OPEN_SQUARE_BRACKET"] = 87] = "US_OPEN_SQUARE_BRACKET";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the '\|' key
         */
        KeyCode[KeyCode["US_BACKSLASH"] = 88] = "US_BACKSLASH";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the ']}' key
         */
        KeyCode[KeyCode["US_CLOSE_SQUARE_BRACKET"] = 89] = "US_CLOSE_SQUARE_BRACKET";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         * For the US standard keyboard, the ''"' key
         */
        KeyCode[KeyCode["US_QUOTE"] = 90] = "US_QUOTE";
        /**
         * Used for miscellaneous characters; it can vary by keyboard.
         */
        KeyCode[KeyCode["OEM_8"] = 91] = "OEM_8";
        /**
         * Either the angle bracket key or the backslash key on the RT 102-key keyboard.
         */
        KeyCode[KeyCode["OEM_102"] = 92] = "OEM_102";
        KeyCode[KeyCode["NUMPAD_0"] = 93] = "NUMPAD_0";
        KeyCode[KeyCode["NUMPAD_1"] = 94] = "NUMPAD_1";
        KeyCode[KeyCode["NUMPAD_2"] = 95] = "NUMPAD_2";
        KeyCode[KeyCode["NUMPAD_3"] = 96] = "NUMPAD_3";
        KeyCode[KeyCode["NUMPAD_4"] = 97] = "NUMPAD_4";
        KeyCode[KeyCode["NUMPAD_5"] = 98] = "NUMPAD_5";
        KeyCode[KeyCode["NUMPAD_6"] = 99] = "NUMPAD_6";
        KeyCode[KeyCode["NUMPAD_7"] = 100] = "NUMPAD_7";
        KeyCode[KeyCode["NUMPAD_8"] = 101] = "NUMPAD_8";
        KeyCode[KeyCode["NUMPAD_9"] = 102] = "NUMPAD_9";
        KeyCode[KeyCode["NUMPAD_MULTIPLY"] = 103] = "NUMPAD_MULTIPLY";
        KeyCode[KeyCode["NUMPAD_ADD"] = 104] = "NUMPAD_ADD";
        KeyCode[KeyCode["NUMPAD_SEPARATOR"] = 105] = "NUMPAD_SEPARATOR";
        KeyCode[KeyCode["NUMPAD_SUBTRACT"] = 106] = "NUMPAD_SUBTRACT";
        KeyCode[KeyCode["NUMPAD_DECIMAL"] = 107] = "NUMPAD_DECIMAL";
        KeyCode[KeyCode["NUMPAD_DIVIDE"] = 108] = "NUMPAD_DIVIDE";
        /**
         * Cover all key codes when IME is processing input.
         */
        KeyCode[KeyCode["KEY_IN_COMPOSITION"] = 109] = "KEY_IN_COMPOSITION";
        KeyCode[KeyCode["ABNT_C1"] = 110] = "ABNT_C1";
        KeyCode[KeyCode["ABNT_C2"] = 111] = "ABNT_C2";
        /**
         * Placed last to cover the length of the enum.
         * Please do not depend on this value!
         */
        KeyCode[KeyCode["MAX_VALUE"] = 112] = "MAX_VALUE";
    })(KeyCode = exports.KeyCode || (exports.KeyCode = {}));
    var MarkerSeverity;
    (function (MarkerSeverity) {
        MarkerSeverity[MarkerSeverity["Hint"] = 1] = "Hint";
        MarkerSeverity[MarkerSeverity["Info"] = 2] = "Info";
        MarkerSeverity[MarkerSeverity["Warning"] = 4] = "Warning";
        MarkerSeverity[MarkerSeverity["Error"] = 8] = "Error";
    })(MarkerSeverity = exports.MarkerSeverity || (exports.MarkerSeverity = {}));
    var MarkerTag;
    (function (MarkerTag) {
        MarkerTag[MarkerTag["Unnecessary"] = 1] = "Unnecessary";
        MarkerTag[MarkerTag["Deprecated"] = 2] = "Deprecated";
    })(MarkerTag = exports.MarkerTag || (exports.MarkerTag = {}));
    /**
     * Position in the minimap to render the decoration.
     */
    var MinimapPosition;
    (function (MinimapPosition) {
        MinimapPosition[MinimapPosition["Inline"] = 1] = "Inline";
        MinimapPosition[MinimapPosition["Gutter"] = 2] = "Gutter";
    })(MinimapPosition = exports.MinimapPosition || (exports.MinimapPosition = {}));
    /**
     * Type of hit element with the mouse in the editor.
     */
    var MouseTargetType;
    (function (MouseTargetType) {
        /**
         * Mouse is on top of an unknown element.
         */
        MouseTargetType[MouseTargetType["UNKNOWN"] = 0] = "UNKNOWN";
        /**
         * Mouse is on top of the textarea used for input.
         */
        MouseTargetType[MouseTargetType["TEXTAREA"] = 1] = "TEXTAREA";
        /**
         * Mouse is on top of the glyph margin
         */
        MouseTargetType[MouseTargetType["GUTTER_GLYPH_MARGIN"] = 2] = "GUTTER_GLYPH_MARGIN";
        /**
         * Mouse is on top of the line numbers
         */
        MouseTargetType[MouseTargetType["GUTTER_LINE_NUMBERS"] = 3] = "GUTTER_LINE_NUMBERS";
        /**
         * Mouse is on top of the line decorations
         */
        MouseTargetType[MouseTargetType["GUTTER_LINE_DECORATIONS"] = 4] = "GUTTER_LINE_DECORATIONS";
        /**
         * Mouse is on top of the whitespace left in the gutter by a view zone.
         */
        MouseTargetType[MouseTargetType["GUTTER_VIEW_ZONE"] = 5] = "GUTTER_VIEW_ZONE";
        /**
         * Mouse is on top of text in the content.
         */
        MouseTargetType[MouseTargetType["CONTENT_TEXT"] = 6] = "CONTENT_TEXT";
        /**
         * Mouse is on top of empty space in the content (e.g. after line text or below last line)
         */
        MouseTargetType[MouseTargetType["CONTENT_EMPTY"] = 7] = "CONTENT_EMPTY";
        /**
         * Mouse is on top of a view zone in the content.
         */
        MouseTargetType[MouseTargetType["CONTENT_VIEW_ZONE"] = 8] = "CONTENT_VIEW_ZONE";
        /**
         * Mouse is on top of a content widget.
         */
        MouseTargetType[MouseTargetType["CONTENT_WIDGET"] = 9] = "CONTENT_WIDGET";
        /**
         * Mouse is on top of the decorations overview ruler.
         */
        MouseTargetType[MouseTargetType["OVERVIEW_RULER"] = 10] = "OVERVIEW_RULER";
        /**
         * Mouse is on top of a scrollbar.
         */
        MouseTargetType[MouseTargetType["SCROLLBAR"] = 11] = "SCROLLBAR";
        /**
         * Mouse is on top of an overlay widget.
         */
        MouseTargetType[MouseTargetType["OVERLAY_WIDGET"] = 12] = "OVERLAY_WIDGET";
        /**
         * Mouse is outside of the editor.
         */
        MouseTargetType[MouseTargetType["OUTSIDE_EDITOR"] = 13] = "OUTSIDE_EDITOR";
    })(MouseTargetType = exports.MouseTargetType || (exports.MouseTargetType = {}));
    /**
     * A positioning preference for rendering overlay widgets.
     */
    var OverlayWidgetPositionPreference;
    (function (OverlayWidgetPositionPreference) {
        /**
         * Position the overlay widget in the top right corner
         */
        OverlayWidgetPositionPreference[OverlayWidgetPositionPreference["TOP_RIGHT_CORNER"] = 0] = "TOP_RIGHT_CORNER";
        /**
         * Position the overlay widget in the bottom right corner
         */
        OverlayWidgetPositionPreference[OverlayWidgetPositionPreference["BOTTOM_RIGHT_CORNER"] = 1] = "BOTTOM_RIGHT_CORNER";
        /**
         * Position the overlay widget in the top center
         */
        OverlayWidgetPositionPreference[OverlayWidgetPositionPreference["TOP_CENTER"] = 2] = "TOP_CENTER";
    })(OverlayWidgetPositionPreference = exports.OverlayWidgetPositionPreference || (exports.OverlayWidgetPositionPreference = {}));
    /**
     * Vertical Lane in the overview ruler of the editor.
     */
    var OverviewRulerLane;
    (function (OverviewRulerLane) {
        OverviewRulerLane[OverviewRulerLane["Left"] = 1] = "Left";
        OverviewRulerLane[OverviewRulerLane["Center"] = 2] = "Center";
        OverviewRulerLane[OverviewRulerLane["Right"] = 4] = "Right";
        OverviewRulerLane[OverviewRulerLane["Full"] = 7] = "Full";
    })(OverviewRulerLane = exports.OverviewRulerLane || (exports.OverviewRulerLane = {}));
    var RenderLineNumbersType;
    (function (RenderLineNumbersType) {
        RenderLineNumbersType[RenderLineNumbersType["Off"] = 0] = "Off";
        RenderLineNumbersType[RenderLineNumbersType["On"] = 1] = "On";
        RenderLineNumbersType[RenderLineNumbersType["Relative"] = 2] = "Relative";
        RenderLineNumbersType[RenderLineNumbersType["Interval"] = 3] = "Interval";
        RenderLineNumbersType[RenderLineNumbersType["Custom"] = 4] = "Custom";
    })(RenderLineNumbersType = exports.RenderLineNumbersType || (exports.RenderLineNumbersType = {}));
    var RenderMinimap;
    (function (RenderMinimap) {
        RenderMinimap[RenderMinimap["None"] = 0] = "None";
        RenderMinimap[RenderMinimap["Text"] = 1] = "Text";
        RenderMinimap[RenderMinimap["Blocks"] = 2] = "Blocks";
    })(RenderMinimap = exports.RenderMinimap || (exports.RenderMinimap = {}));
    var ScrollType;
    (function (ScrollType) {
        ScrollType[ScrollType["Smooth"] = 0] = "Smooth";
        ScrollType[ScrollType["Immediate"] = 1] = "Immediate";
    })(ScrollType = exports.ScrollType || (exports.ScrollType = {}));
    var ScrollbarVisibility;
    (function (ScrollbarVisibility) {
        ScrollbarVisibility[ScrollbarVisibility["Auto"] = 1] = "Auto";
        ScrollbarVisibility[ScrollbarVisibility["Hidden"] = 2] = "Hidden";
        ScrollbarVisibility[ScrollbarVisibility["Visible"] = 3] = "Visible";
    })(ScrollbarVisibility = exports.ScrollbarVisibility || (exports.ScrollbarVisibility = {}));
    /**
     * The direction of a selection.
     */
    var SelectionDirection;
    (function (SelectionDirection) {
        /**
         * The selection starts above where it ends.
         */
        SelectionDirection[SelectionDirection["LTR"] = 0] = "LTR";
        /**
         * The selection starts below where it ends.
         */
        SelectionDirection[SelectionDirection["RTL"] = 1] = "RTL";
    })(SelectionDirection = exports.SelectionDirection || (exports.SelectionDirection = {}));
    var SignatureHelpTriggerKind;
    (function (SignatureHelpTriggerKind) {
        SignatureHelpTriggerKind[SignatureHelpTriggerKind["Invoke"] = 1] = "Invoke";
        SignatureHelpTriggerKind[SignatureHelpTriggerKind["TriggerCharacter"] = 2] = "TriggerCharacter";
        SignatureHelpTriggerKind[SignatureHelpTriggerKind["ContentChange"] = 3] = "ContentChange";
    })(SignatureHelpTriggerKind = exports.SignatureHelpTriggerKind || (exports.SignatureHelpTriggerKind = {}));
    /**
     * A symbol kind.
     */
    var SymbolKind;
    (function (SymbolKind) {
        SymbolKind[SymbolKind["File"] = 0] = "File";
        SymbolKind[SymbolKind["Module"] = 1] = "Module";
        SymbolKind[SymbolKind["Namespace"] = 2] = "Namespace";
        SymbolKind[SymbolKind["Package"] = 3] = "Package";
        SymbolKind[SymbolKind["Class"] = 4] = "Class";
        SymbolKind[SymbolKind["Method"] = 5] = "Method";
        SymbolKind[SymbolKind["Property"] = 6] = "Property";
        SymbolKind[SymbolKind["Field"] = 7] = "Field";
        SymbolKind[SymbolKind["Constructor"] = 8] = "Constructor";
        SymbolKind[SymbolKind["Enum"] = 9] = "Enum";
        SymbolKind[SymbolKind["Interface"] = 10] = "Interface";
        SymbolKind[SymbolKind["Function"] = 11] = "Function";
        SymbolKind[SymbolKind["Variable"] = 12] = "Variable";
        SymbolKind[SymbolKind["Constant"] = 13] = "Constant";
        SymbolKind[SymbolKind["String"] = 14] = "String";
        SymbolKind[SymbolKind["Number"] = 15] = "Number";
        SymbolKind[SymbolKind["Boolean"] = 16] = "Boolean";
        SymbolKind[SymbolKind["Array"] = 17] = "Array";
        SymbolKind[SymbolKind["Object"] = 18] = "Object";
        SymbolKind[SymbolKind["Key"] = 19] = "Key";
        SymbolKind[SymbolKind["Null"] = 20] = "Null";
        SymbolKind[SymbolKind["EnumMember"] = 21] = "EnumMember";
        SymbolKind[SymbolKind["Struct"] = 22] = "Struct";
        SymbolKind[SymbolKind["Event"] = 23] = "Event";
        SymbolKind[SymbolKind["Operator"] = 24] = "Operator";
        SymbolKind[SymbolKind["TypeParameter"] = 25] = "TypeParameter";
    })(SymbolKind = exports.SymbolKind || (exports.SymbolKind = {}));
    var SymbolTag;
    (function (SymbolTag) {
        SymbolTag[SymbolTag["Deprecated"] = 1] = "Deprecated";
    })(SymbolTag = exports.SymbolTag || (exports.SymbolTag = {}));
    /**
     * The kind of animation in which the editor's cursor should be rendered.
     */
    var TextEditorCursorBlinkingStyle;
    (function (TextEditorCursorBlinkingStyle) {
        /**
         * Hidden
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Hidden"] = 0] = "Hidden";
        /**
         * Blinking
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Blink"] = 1] = "Blink";
        /**
         * Blinking with smooth fading
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Smooth"] = 2] = "Smooth";
        /**
         * Blinking with prolonged filled state and smooth fading
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Phase"] = 3] = "Phase";
        /**
         * Expand collapse animation on the y axis
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Expand"] = 4] = "Expand";
        /**
         * No-Blinking
         */
        TextEditorCursorBlinkingStyle[TextEditorCursorBlinkingStyle["Solid"] = 5] = "Solid";
    })(TextEditorCursorBlinkingStyle = exports.TextEditorCursorBlinkingStyle || (exports.TextEditorCursorBlinkingStyle = {}));
    /**
     * The style in which the editor's cursor should be rendered.
     */
    var TextEditorCursorStyle;
    (function (TextEditorCursorStyle) {
        /**
         * As a vertical line (sitting between two characters).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["Line"] = 1] = "Line";
        /**
         * As a block (sitting on top of a character).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["Block"] = 2] = "Block";
        /**
         * As a horizontal line (sitting under a character).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["Underline"] = 3] = "Underline";
        /**
         * As a thin vertical line (sitting between two characters).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["LineThin"] = 4] = "LineThin";
        /**
         * As an outlined block (sitting on top of a character).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["BlockOutline"] = 5] = "BlockOutline";
        /**
         * As a thin horizontal line (sitting under a character).
         */
        TextEditorCursorStyle[TextEditorCursorStyle["UnderlineThin"] = 6] = "UnderlineThin";
    })(TextEditorCursorStyle = exports.TextEditorCursorStyle || (exports.TextEditorCursorStyle = {}));
    /**
     * Describes the behavior of decorations when typing/editing near their edges.
     * Note: Please do not edit the values, as they very carefully match `DecorationRangeBehavior`
     */
    var TrackedRangeStickiness;
    (function (TrackedRangeStickiness) {
        TrackedRangeStickiness[TrackedRangeStickiness["AlwaysGrowsWhenTypingAtEdges"] = 0] = "AlwaysGrowsWhenTypingAtEdges";
        TrackedRangeStickiness[TrackedRangeStickiness["NeverGrowsWhenTypingAtEdges"] = 1] = "NeverGrowsWhenTypingAtEdges";
        TrackedRangeStickiness[TrackedRangeStickiness["GrowsOnlyWhenTypingBefore"] = 2] = "GrowsOnlyWhenTypingBefore";
        TrackedRangeStickiness[TrackedRangeStickiness["GrowsOnlyWhenTypingAfter"] = 3] = "GrowsOnlyWhenTypingAfter";
    })(TrackedRangeStickiness = exports.TrackedRangeStickiness || (exports.TrackedRangeStickiness = {}));
    /**
     * Describes how to indent wrapped lines.
     */
    var WrappingIndent;
    (function (WrappingIndent) {
        /**
         * No indentation => wrapped lines begin at column 1.
         */
        WrappingIndent[WrappingIndent["None"] = 0] = "None";
        /**
         * Same => wrapped lines get the same indentation as the parent.
         */
        WrappingIndent[WrappingIndent["Same"] = 1] = "Same";
        /**
         * Indent => wrapped lines get +1 indentation toward the parent.
         */
        WrappingIndent[WrappingIndent["Indent"] = 2] = "Indent";
        /**
         * DeepIndent => wrapped lines get +2 indentation toward the parent.
         */
        WrappingIndent[WrappingIndent["DeepIndent"] = 3] = "DeepIndent";
    })(WrappingIndent = exports.WrappingIndent || (exports.WrappingIndent = {}));
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[30/*vs/editor/common/standalone/standaloneBase*/], __M([1/*require*/,0/*exports*/,22/*vs/base/common/cancellation*/,13/*vs/base/common/event*/,16/*vs/base/common/keyCodes*/,11/*vs/base/common/uri*/,2/*vs/editor/common/core/position*/,4/*vs/editor/common/core/range*/,23/*vs/editor/common/core/selection*/,24/*vs/editor/common/core/token*/,29/*vs/editor/common/standalone/standaloneEnums*/,32/*vs/editor/common/standalone/promise-polyfill/polyfill*/]), function (require, exports, cancellation_1, event_1, keyCodes_1, uri_1, position_1, range_1, selection_1, token_1, standaloneEnums) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var KeyMod = /** @class */ (function () {
        function KeyMod() {
        }
        KeyMod.chord = function (firstPart, secondPart) {
            return keyCodes_1.KeyChord(firstPart, secondPart);
        };
        KeyMod.CtrlCmd = 2048 /* CtrlCmd */;
        KeyMod.Shift = 1024 /* Shift */;
        KeyMod.Alt = 512 /* Alt */;
        KeyMod.WinCtrl = 256 /* WinCtrl */;
        return KeyMod;
    }());
    exports.KeyMod = KeyMod;
    function createMonacoBaseAPI() {
        return {
            editor: undefined,
            languages: undefined,
            CancellationTokenSource: cancellation_1.CancellationTokenSource,
            Emitter: event_1.Emitter,
            KeyCode: standaloneEnums.KeyCode,
            KeyMod: KeyMod,
            Position: position_1.Position,
            Range: range_1.Range,
            Selection: selection_1.Selection,
            SelectionDirection: standaloneEnums.SelectionDirection,
            MarkerSeverity: standaloneEnums.MarkerSeverity,
            MarkerTag: standaloneEnums.MarkerTag,
            Uri: uri_1.URI,
            Token: token_1.Token
        };
    }
    exports.createMonacoBaseAPI = createMonacoBaseAPI;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[31/*vs/editor/common/viewModel/prefixSumComputer*/], __M([1/*require*/,0/*exports*/,7/*vs/base/common/uint*/]), function (require, exports, uint_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var PrefixSumIndexOfResult = /** @class */ (function () {
        function PrefixSumIndexOfResult(index, remainder) {
            this.index = index;
            this.remainder = remainder;
        }
        return PrefixSumIndexOfResult;
    }());
    exports.PrefixSumIndexOfResult = PrefixSumIndexOfResult;
    var PrefixSumComputer = /** @class */ (function () {
        function PrefixSumComputer(values) {
            this.values = values;
            this.prefixSum = new Uint32Array(values.length);
            this.prefixSumValidIndex = new Int32Array(1);
            this.prefixSumValidIndex[0] = -1;
        }
        PrefixSumComputer.prototype.getCount = function () {
            return this.values.length;
        };
        PrefixSumComputer.prototype.insertValues = function (insertIndex, insertValues) {
            insertIndex = uint_1.toUint32(insertIndex);
            var oldValues = this.values;
            var oldPrefixSum = this.prefixSum;
            var insertValuesLen = insertValues.length;
            if (insertValuesLen === 0) {
                return false;
            }
            this.values = new Uint32Array(oldValues.length + insertValuesLen);
            this.values.set(oldValues.subarray(0, insertIndex), 0);
            this.values.set(oldValues.subarray(insertIndex), insertIndex + insertValuesLen);
            this.values.set(insertValues, insertIndex);
            if (insertIndex - 1 < this.prefixSumValidIndex[0]) {
                this.prefixSumValidIndex[0] = insertIndex - 1;
            }
            this.prefixSum = new Uint32Array(this.values.length);
            if (this.prefixSumValidIndex[0] >= 0) {
                this.prefixSum.set(oldPrefixSum.subarray(0, this.prefixSumValidIndex[0] + 1));
            }
            return true;
        };
        PrefixSumComputer.prototype.changeValue = function (index, value) {
            index = uint_1.toUint32(index);
            value = uint_1.toUint32(value);
            if (this.values[index] === value) {
                return false;
            }
            this.values[index] = value;
            if (index - 1 < this.prefixSumValidIndex[0]) {
                this.prefixSumValidIndex[0] = index - 1;
            }
            return true;
        };
        PrefixSumComputer.prototype.removeValues = function (startIndex, cnt) {
            startIndex = uint_1.toUint32(startIndex);
            cnt = uint_1.toUint32(cnt);
            var oldValues = this.values;
            var oldPrefixSum = this.prefixSum;
            if (startIndex >= oldValues.length) {
                return false;
            }
            var maxCnt = oldValues.length - startIndex;
            if (cnt >= maxCnt) {
                cnt = maxCnt;
            }
            if (cnt === 0) {
                return false;
            }
            this.values = new Uint32Array(oldValues.length - cnt);
            this.values.set(oldValues.subarray(0, startIndex), 0);
            this.values.set(oldValues.subarray(startIndex + cnt), startIndex);
            this.prefixSum = new Uint32Array(this.values.length);
            if (startIndex - 1 < this.prefixSumValidIndex[0]) {
                this.prefixSumValidIndex[0] = startIndex - 1;
            }
            if (this.prefixSumValidIndex[0] >= 0) {
                this.prefixSum.set(oldPrefixSum.subarray(0, this.prefixSumValidIndex[0] + 1));
            }
            return true;
        };
        PrefixSumComputer.prototype.getTotalValue = function () {
            if (this.values.length === 0) {
                return 0;
            }
            return this._getAccumulatedValue(this.values.length - 1);
        };
        PrefixSumComputer.prototype.getAccumulatedValue = function (index) {
            if (index < 0) {
                return 0;
            }
            index = uint_1.toUint32(index);
            return this._getAccumulatedValue(index);
        };
        PrefixSumComputer.prototype._getAccumulatedValue = function (index) {
            if (index <= this.prefixSumValidIndex[0]) {
                return this.prefixSum[index];
            }
            var startIndex = this.prefixSumValidIndex[0] + 1;
            if (startIndex === 0) {
                this.prefixSum[0] = this.values[0];
                startIndex++;
            }
            if (index >= this.values.length) {
                index = this.values.length - 1;
            }
            for (var i = startIndex; i <= index; i++) {
                this.prefixSum[i] = this.prefixSum[i - 1] + this.values[i];
            }
            this.prefixSumValidIndex[0] = Math.max(this.prefixSumValidIndex[0], index);
            return this.prefixSum[index];
        };
        PrefixSumComputer.prototype.getIndexOf = function (accumulatedValue) {
            accumulatedValue = Math.floor(accumulatedValue); //@perf
            // Compute all sums (to get a fully valid prefixSum)
            this.getTotalValue();
            var low = 0;
            var high = this.values.length - 1;
            var mid = 0;
            var midStop = 0;
            var midStart = 0;
            while (low <= high) {
                mid = low + ((high - low) / 2) | 0;
                midStop = this.prefixSum[mid];
                midStart = midStop - this.values[mid];
                if (accumulatedValue < midStart) {
                    high = mid - 1;
                }
                else if (accumulatedValue >= midStop) {
                    low = mid + 1;
                }
                else {
                    break;
                }
            }
            return new PrefixSumIndexOfResult(mid, accumulatedValue - midStart);
        };
        return PrefixSumComputer;
    }());
    exports.PrefixSumComputer = PrefixSumComputer;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
define(__m[19/*vs/editor/common/model/mirrorTextModel*/], __M([1/*require*/,0/*exports*/,2/*vs/editor/common/core/position*/,31/*vs/editor/common/viewModel/prefixSumComputer*/]), function (require, exports, position_1, prefixSumComputer_1) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    var MirrorTextModel = /** @class */ (function () {
        function MirrorTextModel(uri, lines, eol, versionId) {
            this._uri = uri;
            this._lines = lines;
            this._eol = eol;
            this._versionId = versionId;
            this._lineStarts = null;
        }
        MirrorTextModel.prototype.dispose = function () {
            this._lines.length = 0;
        };
        Object.defineProperty(MirrorTextModel.prototype, "version", {
            get: function () {
                return this._versionId;
            },
            enumerable: true,
            configurable: true
        });
        MirrorTextModel.prototype.getText = function () {
            return this._lines.join(this._eol);
        };
        MirrorTextModel.prototype.onEvents = function (e) {
            if (e.eol && e.eol !== this._eol) {
                this._eol = e.eol;
                this._lineStarts = null;
            }
            // Update my lines
            var changes = e.changes;
            for (var _i = 0, changes_1 = changes; _i < changes_1.length; _i++) {
                var change = changes_1[_i];
                this._acceptDeleteRange(change.range);
                this._acceptInsertText(new position_1.Position(change.range.startLineNumber, change.range.startColumn), change.text);
            }
            this._versionId = e.versionId;
        };
        MirrorTextModel.prototype._ensureLineStarts = function () {
            if (!this._lineStarts) {
                var eolLength = this._eol.length;
                var linesLength = this._lines.length;
                var lineStartValues = new Uint32Array(linesLength);
                for (var i = 0; i < linesLength; i++) {
                    lineStartValues[i] = this._lines[i].length + eolLength;
                }
                this._lineStarts = new prefixSumComputer_1.PrefixSumComputer(lineStartValues);
            }
        };
        /**
         * All changes to a line's text go through this method
         */
        MirrorTextModel.prototype._setLineText = function (lineIndex, newValue) {
            this._lines[lineIndex] = newValue;
            if (this._lineStarts) {
                // update prefix sum
                this._lineStarts.changeValue(lineIndex, this._lines[lineIndex].length + this._eol.length);
            }
        };
        MirrorTextModel.prototype._acceptDeleteRange = function (range) {
            if (range.startLineNumber === range.endLineNumber) {
                if (range.startColumn === range.endColumn) {
                    // Nothing to delete
                    return;
                }
                // Delete text on the affected line
                this._setLineText(range.startLineNumber - 1, this._lines[range.startLineNumber - 1].substring(0, range.startColumn - 1)
                    + this._lines[range.startLineNumber - 1].substring(range.endColumn - 1));
                return;
            }
            // Take remaining text on last line and append it to remaining text on first line
            this._setLineText(range.startLineNumber - 1, this._lines[range.startLineNumber - 1].substring(0, range.startColumn - 1)
                + this._lines[range.endLineNumber - 1].substring(range.endColumn - 1));
            // Delete middle lines
            this._lines.splice(range.startLineNumber, range.endLineNumber - range.startLineNumber);
            if (this._lineStarts) {
                // update prefix sum
                this._lineStarts.removeValues(range.startLineNumber, range.endLineNumber - range.startLineNumber);
            }
        };
        MirrorTextModel.prototype._acceptInsertText = function (position, insertText) {
            if (insertText.length === 0) {
                // Nothing to insert
                return;
            }
            var insertLines = insertText.split(/\r\n|\r|\n/);
            if (insertLines.length === 1) {
                // Inserting text on one line
                this._setLineText(position.lineNumber - 1, this._lines[position.lineNumber - 1].substring(0, position.column - 1)
                    + insertLines[0]
                    + this._lines[position.lineNumber - 1].substring(position.column - 1));
                return;
            }
            // Append overflowing text from first line to the end of text to insert
            insertLines[insertLines.length - 1] += this._lines[position.lineNumber - 1].substring(position.column - 1);
            // Delete overflowing text from first line and insert text on first line
            this._setLineText(position.lineNumber - 1, this._lines[position.lineNumber - 1].substring(0, position.column - 1)
                + insertLines[0]);
            // Insert new lines & store lengths
            var newLengths = new Uint32Array(insertLines.length - 1);
            for (var i = 1; i < insertLines.length; i++) {
                this._lines.splice(position.lineNumber + i - 1, 0, insertLines[i]);
                newLengths[i - 1] = insertLines[i].length + this._eol.length;
            }
            if (this._lineStarts) {
                // update prefix sum
                this._lineStarts.insertValues(position.lineNumber, newLengths);
            }
        };
        return MirrorTextModel;
    }());
    exports.MirrorTextModel = MirrorTextModel;
});

/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

















































define(__m[34/*vs/editor/common/services/editorSimpleWorker*/], __M([1/*require*/,0/*exports*/,21/*vs/base/common/arrays*/,10/*vs/base/common/diff/diff*/,12/*vs/base/common/iterator*/,5/*vs/base/common/platform*/,11/*vs/base/common/uri*/,2/*vs/editor/common/core/position*/,4/*vs/editor/common/core/range*/,25/*vs/editor/common/diff/diffComputer*/,19/*vs/editor/common/model/mirrorTextModel*/,26/*vs/editor/common/model/wordHelper*/,27/*vs/editor/common/modes/linkComputer*/,28/*vs/editor/common/modes/supports/inplaceReplaceSupport*/,30/*vs/editor/common/standalone/standaloneBase*/,9/*vs/base/common/types*/]), function (require, exports, arrays_1, diff_1, iterator_1, platform_1, uri_1, position_1, range_1, diffComputer_1, mirrorTextModel_1, wordHelper_1, linkComputer_1, inplaceReplaceSupport_1, standaloneBase_1, types) {
    "use strict";
    Object.defineProperty(exports, "__esModule", { value: true });
    /**
     * @internal
     */
    var MirrorModel = /** @class */ (function (_super) {
        __extends(MirrorModel, _super);
        function MirrorModel() {
            return _super !== null && _super.apply(this, arguments) || this;
        }
        Object.defineProperty(MirrorModel.prototype, "uri", {
            get: function () {
                return this._uri;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(MirrorModel.prototype, "version", {
            get: function () {
                return this._versionId;
            },
            enumerable: true,
            configurable: true
        });
        Object.defineProperty(MirrorModel.prototype, "eol", {
            get: function () {
                return this._eol;
            },
            enumerable: true,
            configurable: true
        });
        MirrorModel.prototype.getValue = function () {
            return this.getText();
        };
        MirrorModel.prototype.getLinesContent = function () {
            return this._lines.slice(0);
        };
        MirrorModel.prototype.getLineCount = function () {
            return this._lines.length;
        };
        MirrorModel.prototype.getLineContent = function (lineNumber) {
            return this._lines[lineNumber - 1];
        };
        MirrorModel.prototype.getWordAtPosition = function (position, wordDefinition) {
            var wordAtText = wordHelper_1.getWordAtText(position.column, wordHelper_1.ensureValidWordDefinition(wordDefinition), this._lines[position.lineNumber - 1], 0);
            if (wordAtText) {
                return new range_1.Range(position.lineNumber, wordAtText.startColumn, position.lineNumber, wordAtText.endColumn);
            }
            return null;
        };
        MirrorModel.prototype.getWordUntilPosition = function (position, wordDefinition) {
            var wordAtPosition = this.getWordAtPosition(position, wordDefinition);
            if (!wordAtPosition) {
                return {
                    word: '',
                    startColumn: position.column,
                    endColumn: position.column
                };
            }
            return {
                word: this._lines[position.lineNumber - 1].substring(wordAtPosition.startColumn - 1, position.column - 1),
                startColumn: wordAtPosition.startColumn,
                endColumn: position.column
            };
        };
        MirrorModel.prototype.createWordIterator = function (wordDefinition) {
            var _this = this;
            var obj;
            var lineNumber = 0;
            var lineText;
            var wordRangesIdx = 0;
            var wordRanges = [];
            var next = function () {
                if (wordRangesIdx < wordRanges.length) {
                    var value = lineText.substring(wordRanges[wordRangesIdx].start, wordRanges[wordRangesIdx].end);
                    wordRangesIdx += 1;
                    if (!obj) {
                        obj = { done: false, value: value };
                    }
                    else {
                        obj.value = value;
                    }
                    return obj;
                }
                else if (lineNumber >= _this._lines.length) {
                    return iterator_1.FIN;
                }
                else {
                    lineText = _this._lines[lineNumber];
                    wordRanges = _this._wordenize(lineText, wordDefinition);
                    wordRangesIdx = 0;
                    lineNumber += 1;
                    return next();
                }
            };
            return { next: next };
        };
        MirrorModel.prototype.getLineWords = function (lineNumber, wordDefinition) {
            var content = this._lines[lineNumber - 1];
            var ranges = this._wordenize(content, wordDefinition);
            var words = [];
            for (var _i = 0, ranges_1 = ranges; _i < ranges_1.length; _i++) {
                var range = ranges_1[_i];
                words.push({
                    word: content.substring(range.start, range.end),
                    startColumn: range.start + 1,
                    endColumn: range.end + 1
                });
            }
            return words;
        };
        MirrorModel.prototype._wordenize = function (content, wordDefinition) {
            var result = [];
            var match;
            wordDefinition.lastIndex = 0; // reset lastIndex just to be sure
            while (match = wordDefinition.exec(content)) {
                if (match[0].length === 0) {
                    // it did match the empty string
                    break;
                }
                result.push({ start: match.index, end: match.index + match[0].length });
            }
            return result;
        };
        MirrorModel.prototype.getValueInRange = function (range) {
            range = this._validateRange(range);
            if (range.startLineNumber === range.endLineNumber) {
                return this._lines[range.startLineNumber - 1].substring(range.startColumn - 1, range.endColumn - 1);
            }
            var lineEnding = this._eol;
            var startLineIndex = range.startLineNumber - 1;
            var endLineIndex = range.endLineNumber - 1;
            var resultLines = [];
            resultLines.push(this._lines[startLineIndex].substring(range.startColumn - 1));
            for (var i = startLineIndex + 1; i < endLineIndex; i++) {
                resultLines.push(this._lines[i]);
            }
            resultLines.push(this._lines[endLineIndex].substring(0, range.endColumn - 1));
            return resultLines.join(lineEnding);
        };
        MirrorModel.prototype.offsetAt = function (position) {
            position = this._validatePosition(position);
            this._ensureLineStarts();
            return this._lineStarts.getAccumulatedValue(position.lineNumber - 2) + (position.column - 1);
        };
        MirrorModel.prototype.positionAt = function (offset) {
            offset = Math.floor(offset);
            offset = Math.max(0, offset);
            this._ensureLineStarts();
            var out = this._lineStarts.getIndexOf(offset);
            var lineLength = this._lines[out.index].length;
            // Ensure we return a valid position
            return {
                lineNumber: 1 + out.index,
                column: 1 + Math.min(out.remainder, lineLength)
            };
        };
        MirrorModel.prototype._validateRange = function (range) {
            var start = this._validatePosition({ lineNumber: range.startLineNumber, column: range.startColumn });
            var end = this._validatePosition({ lineNumber: range.endLineNumber, column: range.endColumn });
            if (start.lineNumber !== range.startLineNumber
                || start.column !== range.startColumn
                || end.lineNumber !== range.endLineNumber
                || end.column !== range.endColumn) {
                return {
                    startLineNumber: start.lineNumber,
                    startColumn: start.column,
                    endLineNumber: end.lineNumber,
                    endColumn: end.column
                };
            }
            return range;
        };
        MirrorModel.prototype._validatePosition = function (position) {
            if (!position_1.Position.isIPosition(position)) {
                throw new Error('bad position');
            }
            var lineNumber = position.lineNumber, column = position.column;
            var hasChanged = false;
            if (lineNumber < 1) {
                lineNumber = 1;
                column = 1;
                hasChanged = true;
            }
            else if (lineNumber > this._lines.length) {
                lineNumber = this._lines.length;
                column = this._lines[lineNumber - 1].length + 1;
                hasChanged = true;
            }
            else {
                var maxCharacter = this._lines[lineNumber - 1].length + 1;
                if (column < 1) {
                    column = 1;
                    hasChanged = true;
                }
                else if (column > maxCharacter) {
                    column = maxCharacter;
                    hasChanged = true;
                }
            }
            if (!hasChanged) {
                return position;
            }
            else {
                return { lineNumber: lineNumber, column: column };
            }
        };
        return MirrorModel;
    }(mirrorTextModel_1.MirrorTextModel));
    /**
     * @internal
     */
    var EditorSimpleWorker = /** @class */ (function () {
        function EditorSimpleWorker(host, foreignModuleFactory) {
            this._host = host;
            this._models = Object.create(null);
            this._foreignModuleFactory = foreignModuleFactory;
            this._foreignModule = null;
        }
        EditorSimpleWorker.prototype.dispose = function () {
            this._models = Object.create(null);
        };
        EditorSimpleWorker.prototype._getModel = function (uri) {
            return this._models[uri];
        };
        EditorSimpleWorker.prototype._getModels = function () {
            var _this = this;
            var all = [];
            Object.keys(this._models).forEach(function (key) { return all.push(_this._models[key]); });
            return all;
        };
        EditorSimpleWorker.prototype.acceptNewModel = function (data) {
            this._models[data.url] = new MirrorModel(uri_1.URI.parse(data.url), data.lines, data.EOL, data.versionId);
        };
        EditorSimpleWorker.prototype.acceptModelChanged = function (strURL, e) {
            if (!this._models[strURL]) {
                return;
            }
            var model = this._models[strURL];
            model.onEvents(e);
        };
        EditorSimpleWorker.prototype.acceptRemovedModel = function (strURL) {
            if (!this._models[strURL]) {
                return;
            }
            delete this._models[strURL];
        };
        // ---- BEGIN diff --------------------------------------------------------------------------
        EditorSimpleWorker.prototype.computeDiff = function (originalUrl, modifiedUrl, ignoreTrimWhitespace, maxComputationTime) {
            return __awaiter(this, void 0, void 0, function () {
                var original, modified, originalLines, modifiedLines, diffComputer, diffResult, identical;
                return __generator(this, function (_a) {
                    original = this._getModel(originalUrl);
                    modified = this._getModel(modifiedUrl);
                    if (!original || !modified) {
                        return [2 /*return*/, null];
                    }
                    originalLines = original.getLinesContent();
                    modifiedLines = modified.getLinesContent();
                    diffComputer = new diffComputer_1.DiffComputer(originalLines, modifiedLines, {
                        shouldComputeCharChanges: true,
                        shouldPostProcessCharChanges: true,
                        shouldIgnoreTrimWhitespace: ignoreTrimWhitespace,
                        shouldMakePrettyDiff: true,
                        maxComputationTime: maxComputationTime
                    });
                    diffResult = diffComputer.computeDiff();
                    identical = (diffResult.changes.length > 0 ? false : this._modelsAreIdentical(original, modified));
                    return [2 /*return*/, {
                            quitEarly: diffResult.quitEarly,
                            identical: identical,
                            changes: diffResult.changes
                        }];
                });
            });
        };
        EditorSimpleWorker.prototype._modelsAreIdentical = function (original, modified) {
            var originalLineCount = original.getLineCount();
            var modifiedLineCount = modified.getLineCount();
            if (originalLineCount !== modifiedLineCount) {
                return false;
            }
            for (var line = 1; line <= originalLineCount; line++) {
                var originalLine = original.getLineContent(line);
                var modifiedLine = modified.getLineContent(line);
                if (originalLine !== modifiedLine) {
                    return false;
                }
            }
            return true;
        };
        EditorSimpleWorker.prototype.computeDirtyDiff = function (originalUrl, modifiedUrl, ignoreTrimWhitespace) {
            return __awaiter(this, void 0, void 0, function () {
                var original, modified, originalLines, modifiedLines, diffComputer;
                return __generator(this, function (_a) {
                    original = this._getModel(originalUrl);
                    modified = this._getModel(modifiedUrl);
                    if (!original || !modified) {
                        return [2 /*return*/, null];
                    }
                    originalLines = original.getLinesContent();
                    modifiedLines = modified.getLinesContent();
                    diffComputer = new diffComputer_1.DiffComputer(originalLines, modifiedLines, {
                        shouldComputeCharChanges: false,
                        shouldPostProcessCharChanges: false,
                        shouldIgnoreTrimWhitespace: ignoreTrimWhitespace,
                        shouldMakePrettyDiff: true,
                        maxComputationTime: 1000
                    });
                    return [2 /*return*/, diffComputer.computeDiff().changes];
                });
            });
        };
        EditorSimpleWorker.prototype.computeMoreMinimalEdits = function (modelUrl, edits) {
            return __awaiter(this, void 0, void 0, function () {
                var model, result, lastEol, _i, edits_1, _a, range, text, eol, original, changes, editOffset, _b, changes_1, change, start, end, newEdit;
                return __generator(this, function (_c) {
                    model = this._getModel(modelUrl);
                    if (!model) {
                        return [2 /*return*/, edits];
                    }
                    result = [];
                    lastEol = undefined;
                    edits = arrays_1.mergeSort(edits, function (a, b) {
                        if (a.range && b.range) {
                            return range_1.Range.compareRangesUsingStarts(a.range, b.range);
                        }
                        // eol only changes should go to the end
                        var aRng = a.range ? 0 : 1;
                        var bRng = b.range ? 0 : 1;
                        return aRng - bRng;
                    });
                    for (_i = 0, edits_1 = edits; _i < edits_1.length; _i++) {
                        _a = edits_1[_i], range = _a.range, text = _a.text, eol = _a.eol;
                        if (typeof eol === 'number') {
                            lastEol = eol;
                        }
                        if (range_1.Range.isEmpty(range) && !text) {
                            // empty change
                            continue;
                        }
                        original = model.getValueInRange(range);
                        text = text.replace(/\r\n|\n|\r/g, model.eol);
                        if (original === text) {
                            // noop
                            continue;
                        }
                        // make sure diff won't take too long
                        if (Math.max(text.length, original.length) > EditorSimpleWorker._diffLimit) {
                            result.push({ range: range, text: text });
                            continue;
                        }
                        changes = diff_1.stringDiff(original, text, false);
                        editOffset = model.offsetAt(range_1.Range.lift(range).getStartPosition());
                        for (_b = 0, changes_1 = changes; _b < changes_1.length; _b++) {
                            change = changes_1[_b];
                            start = model.positionAt(editOffset + change.originalStart);
                            end = model.positionAt(editOffset + change.originalStart + change.originalLength);
                            newEdit = {
                                text: text.substr(change.modifiedStart, change.modifiedLength),
                                range: { startLineNumber: start.lineNumber, startColumn: start.column, endLineNumber: end.lineNumber, endColumn: end.column }
                            };
                            if (model.getValueInRange(newEdit.range) !== newEdit.text) {
                                result.push(newEdit);
                            }
                        }
                    }
                    if (typeof lastEol === 'number') {
                        result.push({ eol: lastEol, text: '', range: { startLineNumber: 0, startColumn: 0, endLineNumber: 0, endColumn: 0 } });
                    }
                    return [2 /*return*/, result];
                });
            });
        };
        // ---- END minimal edits ---------------------------------------------------------------
        EditorSimpleWorker.prototype.computeLinks = function (modelUrl) {
            return __awaiter(this, void 0, void 0, function () {
                var model;
                return __generator(this, function (_a) {
                    model = this._getModel(modelUrl);
                    if (!model) {
                        return [2 /*return*/, null];
                    }
                    return [2 /*return*/, linkComputer_1.computeLinks(model)];
                });
            });
        };
        EditorSimpleWorker.prototype.textualSuggest = function (modelUrl, position, wordDef, wordDefFlags) {
            return __awaiter(this, void 0, void 0, function () {
                var model, words, seen, wordDefRegExp, wordAt, iter, e, word;
                return __generator(this, function (_a) {
                    model = this._getModel(modelUrl);
                    if (!model) {
                        return [2 /*return*/, null];
                    }
                    words = [];
                    seen = new Set();
                    wordDefRegExp = new RegExp(wordDef, wordDefFlags);
                    wordAt = model.getWordAtPosition(position, wordDefRegExp);
                    if (wordAt) {
                        seen.add(model.getValueInRange(wordAt));
                    }
                    for (iter = model.createWordIterator(wordDefRegExp), e = iter.next(); !e.done && seen.size <= EditorSimpleWorker._suggestionsLimit; e = iter.next()) {
                        word = e.value;
                        if (seen.has(word)) {
                            continue;
                        }
                        seen.add(word);
                        if (!isNaN(Number(word))) {
                            continue;
                        }
                        words.push(word);
                    }
                    return [2 /*return*/, words];
                });
            });
        };
        // ---- END suggest --------------------------------------------------------------------------
        //#region -- word ranges --
        EditorSimpleWorker.prototype.computeWordRanges = function (modelUrl, range, wordDef, wordDefFlags) {
            return __awaiter(this, void 0, void 0, function () {
                var model, wordDefRegExp, result, line, words, _i, words_1, word, array;
                return __generator(this, function (_a) {
                    model = this._getModel(modelUrl);
                    if (!model) {
                        return [2 /*return*/, Object.create(null)];
                    }
                    wordDefRegExp = new RegExp(wordDef, wordDefFlags);
                    result = Object.create(null);
                    for (line = range.startLineNumber; line < range.endLineNumber; line++) {
                        words = model.getLineWords(line, wordDefRegExp);
                        for (_i = 0, words_1 = words; _i < words_1.length; _i++) {
                            word = words_1[_i];
                            if (!isNaN(Number(word.word))) {
                                continue;
                            }
                            array = result[word.word];
                            if (!array) {
                                array = [];
                                result[word.word] = array;
                            }
                            array.push({
                                startLineNumber: line,
                                startColumn: word.startColumn,
                                endLineNumber: line,
                                endColumn: word.endColumn
                            });
                        }
                    }
                    return [2 /*return*/, result];
                });
            });
        };
        //#endregion
        EditorSimpleWorker.prototype.navigateValueSet = function (modelUrl, range, up, wordDef, wordDefFlags) {
            return __awaiter(this, void 0, void 0, function () {
                var model, wordDefRegExp, selectionText, wordRange, word, result;
                return __generator(this, function (_a) {
                    model = this._getModel(modelUrl);
                    if (!model) {
                        return [2 /*return*/, null];
                    }
                    wordDefRegExp = new RegExp(wordDef, wordDefFlags);
                    if (range.startColumn === range.endColumn) {
                        range = {
                            startLineNumber: range.startLineNumber,
                            startColumn: range.startColumn,
                            endLineNumber: range.endLineNumber,
                            endColumn: range.endColumn + 1
                        };
                    }
                    selectionText = model.getValueInRange(range);
                    wordRange = model.getWordAtPosition({ lineNumber: range.startLineNumber, column: range.startColumn }, wordDefRegExp);
                    if (!wordRange) {
                        return [2 /*return*/, null];
                    }
                    word = model.getValueInRange(wordRange);
                    result = inplaceReplaceSupport_1.BasicInplaceReplace.INSTANCE.navigateValueSet(range, selectionText, wordRange, word, up);
                    return [2 /*return*/, result];
                });
            });
        };
        // ---- BEGIN foreign module support --------------------------------------------------------------------------
        EditorSimpleWorker.prototype.loadForeignModule = function (moduleId, createData, foreignHostMethods) {
            var _this = this;
            var proxyMethodRequest = function (method, args) {
                return _this._host.fhr(method, args);
            };
            var foreignHost = types.createProxyObject(foreignHostMethods, proxyMethodRequest);
            var ctx = {
                host: foreignHost,
                getMirrorModels: function () {
                    return _this._getModels();
                }
            };
            if (this._foreignModuleFactory) {
                this._foreignModule = this._foreignModuleFactory(ctx, createData);
                // static foreing module
                return Promise.resolve(types.getAllMethodNames(this._foreignModule));
            }
            // ESM-comment-begin
            return new Promise(function (resolve, reject) {
                require([moduleId], function (foreignModule) {
                    _this._foreignModule = foreignModule.create(ctx, createData);
                    resolve(types.getAllMethodNames(_this._foreignModule));
                }, reject);
            });
            // ESM-comment-end
            // ESM-uncomment-begin
            // return Promise.reject(new Error(`Unexpected usage`));
            // ESM-uncomment-end
        };
        // foreign method request
        EditorSimpleWorker.prototype.fmr = function (method, args) {
            if (!this._foreignModule || typeof this._foreignModule[method] !== 'function') {
                return Promise.reject(new Error('Missing requestHandler or method: ' + method));
            }
            try {
                return Promise.resolve(this._foreignModule[method].apply(this._foreignModule, args));
            }
            catch (e) {
                return Promise.reject(e);
            }
        };
        // ---- END diff --------------------------------------------------------------------------
        // ---- BEGIN minimal edits ---------------------------------------------------------------
        EditorSimpleWorker._diffLimit = 100000;
        // ---- BEGIN suggest --------------------------------------------------------------------------
        EditorSimpleWorker._suggestionsLimit = 10000;
        return EditorSimpleWorker;
    }());
    exports.EditorSimpleWorker = EditorSimpleWorker;
    /**
     * Called on the worker side
     * @internal
     */
    function create(host) {
        return new EditorSimpleWorker(host, null);
    }
    exports.create = create;
    if (typeof importScripts === 'function') {
        // Running in a web worker
        platform_1.globals.monaco = standaloneBase_1.createMonacoBaseAPI();
    }
});

"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
(function () {
    var MonacoEnvironment = self.MonacoEnvironment;
    var monacoBaseUrl = MonacoEnvironment && MonacoEnvironment.baseUrl ? MonacoEnvironment.baseUrl : '../../../';
    if (typeof self.define !== 'function' || !self.define.amd) {
        importScripts(monacoBaseUrl + 'vs/loader.js');
    }
    require.config({
        baseUrl: monacoBaseUrl,
        catchError: true
    });
    var loadCode = function (moduleId) {
        require([moduleId], function (ws) {
            setTimeout(function () {
                var messageHandler = ws.create(function (msg, transfer) {
                    self.postMessage(msg, transfer);
                }, null);
                self.onmessage = function (e) { return messageHandler.onmessage(e.data); };
                while (beforeReadyMessages.length > 0) {
                    self.onmessage(beforeReadyMessages.shift());
                }
            }, 0);
        });
    };
    var isFirstMessage = true;
    var beforeReadyMessages = [];
    self.onmessage = function (message) {
        if (!isFirstMessage) {
            beforeReadyMessages.push(message);
            return;
        }
        isFirstMessage = false;
        loadCode(message.data);
    };
})();

}).call(this);
//# sourceMappingURL=workerMain.js.map
