/*eslint-disable block-scoped-var, no-redeclare, no-control-regex, no-prototype-builtins*/
"use strict";

var $protobuf = require("protobufjs/minimal");

// Common aliases
var $Reader = $protobuf.Reader, $Writer = $protobuf.Writer, $util = $protobuf.util;

// Exported root namespace
var $root = $protobuf.roots["default"] || ($protobuf.roots["default"] = {});

$root.master = (function() {

    /**
     * Namespace master.
     * @exports master
     * @namespace
     */
    var master = {};

    master.Player = (function() {

        /**
         * Properties of a Player.
         * @memberof master
         * @interface IPlayer
         * @property {string} [name] Player name
         * @property {Array.<string>} [identifiers] Player identifiers
         * @property {string} [endpoint] Player endpoint
         * @property {number} [ping] Player ping
         * @property {number} [id] Player id
         */

        /**
         * Constructs a new Player.
         * @memberof master
         * @classdesc Represents a Player.
         * @constructor
         * @param {master.IPlayer=} [properties] Properties to set
         */
        function Player(properties) {
            this.identifiers = [];
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * Player name.
         * @member {string}name
         * @memberof master.Player
         * @instance
         */
        Player.prototype.name = "";

        /**
         * Player identifiers.
         * @member {Array.<string>}identifiers
         * @memberof master.Player
         * @instance
         */
        Player.prototype.identifiers = $util.emptyArray;

        /**
         * Player endpoint.
         * @member {string}endpoint
         * @memberof master.Player
         * @instance
         */
        Player.prototype.endpoint = "";

        /**
         * Player ping.
         * @member {number}ping
         * @memberof master.Player
         * @instance
         */
        Player.prototype.ping = 0;

        /**
         * Player id.
         * @member {number}id
         * @memberof master.Player
         * @instance
         */
        Player.prototype.id = 0;

        /**
         * Creates a new Player instance using the specified properties.
         * @function create
         * @memberof master.Player
         * @static
         * @param {master.IPlayer=} [properties] Properties to set
         * @returns {master.Player} Player instance
         */
        Player.create = function create(properties) {
            return new Player(properties);
        };

        /**
         * Encodes the specified Player message. Does not implicitly {@link master.Player.verify|verify} messages.
         * @function encode
         * @memberof master.Player
         * @static
         * @param {master.IPlayer} message Player message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Player.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.name != null && message.hasOwnProperty("name"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.name);
            if (message.identifiers != null && message.identifiers.length)
                for (var i = 0; i < message.identifiers.length; ++i)
                    writer.uint32(/* id 2, wireType 2 =*/18).string(message.identifiers[i]);
            if (message.endpoint != null && message.hasOwnProperty("endpoint"))
                writer.uint32(/* id 3, wireType 2 =*/26).string(message.endpoint);
            if (message.ping != null && message.hasOwnProperty("ping"))
                writer.uint32(/* id 4, wireType 0 =*/32).int32(message.ping);
            if (message.id != null && message.hasOwnProperty("id"))
                writer.uint32(/* id 5, wireType 0 =*/40).int32(message.id);
            return writer;
        };

        /**
         * Encodes the specified Player message, length delimited. Does not implicitly {@link master.Player.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.Player
         * @static
         * @param {master.IPlayer} message Player message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Player.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a Player message from the specified reader or buffer.
         * @function decode
         * @memberof master.Player
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Player} Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Player.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.Player();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.name = reader.string();
                    break;
                case 2:
                    if (!(message.identifiers && message.identifiers.length))
                        message.identifiers = [];
                    message.identifiers.push(reader.string());
                    break;
                case 3:
                    message.endpoint = reader.string();
                    break;
                case 4:
                    message.ping = reader.int32();
                    break;
                case 5:
                    message.id = reader.int32();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a Player message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.Player
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Player} Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Player.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a Player message.
         * @function verify
         * @memberof master.Player
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        Player.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.name != null && message.hasOwnProperty("name"))
                if (!$util.isString(message.name))
                    return "name: string expected";
            if (message.identifiers != null && message.hasOwnProperty("identifiers")) {
                if (!Array.isArray(message.identifiers))
                    return "identifiers: array expected";
                for (var i = 0; i < message.identifiers.length; ++i)
                    if (!$util.isString(message.identifiers[i]))
                        return "identifiers: string[] expected";
            }
            if (message.endpoint != null && message.hasOwnProperty("endpoint"))
                if (!$util.isString(message.endpoint))
                    return "endpoint: string expected";
            if (message.ping != null && message.hasOwnProperty("ping"))
                if (!$util.isInteger(message.ping))
                    return "ping: integer expected";
            if (message.id != null && message.hasOwnProperty("id"))
                if (!$util.isInteger(message.id))
                    return "id: integer expected";
            return null;
        };

        /**
         * Creates a Player message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.Player
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Player} Player
         */
        Player.fromObject = function fromObject(object) {
            if (object instanceof $root.master.Player)
                return object;
            var message = new $root.master.Player();
            if (object.name != null)
                message.name = String(object.name);
            if (object.identifiers) {
                if (!Array.isArray(object.identifiers))
                    throw TypeError(".master.Player.identifiers: array expected");
                message.identifiers = [];
                for (var i = 0; i < object.identifiers.length; ++i)
                    message.identifiers[i] = String(object.identifiers[i]);
            }
            if (object.endpoint != null)
                message.endpoint = String(object.endpoint);
            if (object.ping != null)
                message.ping = object.ping | 0;
            if (object.id != null)
                message.id = object.id | 0;
            return message;
        };

        /**
         * Creates a plain object from a Player message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.Player
         * @static
         * @param {master.Player} message Player
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        Player.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.arrays || options.defaults)
                object.identifiers = [];
            if (options.defaults) {
                object.name = "";
                object.endpoint = "";
                object.ping = 0;
                object.id = 0;
            }
            if (message.name != null && message.hasOwnProperty("name"))
                object.name = message.name;
            if (message.identifiers && message.identifiers.length) {
                object.identifiers = [];
                for (var j = 0; j < message.identifiers.length; ++j)
                    object.identifiers[j] = message.identifiers[j];
            }
            if (message.endpoint != null && message.hasOwnProperty("endpoint"))
                object.endpoint = message.endpoint;
            if (message.ping != null && message.hasOwnProperty("ping"))
                object.ping = message.ping;
            if (message.id != null && message.hasOwnProperty("id"))
                object.id = message.id;
            return object;
        };

        /**
         * Converts this Player to JSON.
         * @function toJSON
         * @memberof master.Player
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        Player.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return Player;
    })();

    master.ServerData = (function() {

        /**
         * Properties of a ServerData.
         * @memberof master
         * @interface IServerData
         * @property {number} [svMaxclients] ServerData svMaxclients
         * @property {number} [clients] ServerData clients
         * @property {number} [protocol] ServerData protocol
         * @property {string} [hostname] ServerData hostname
         * @property {string} [gametype] ServerData gametype
         * @property {string} [mapname] ServerData mapname
         * @property {Array.<string>} [resources] ServerData resources
         * @property {string} [server] ServerData server
         * @property {Array.<master.IPlayer>} [players] ServerData players
         * @property {number} [iconVersion] ServerData iconVersion
         * @property {Object.<string,string>} [vars] ServerData vars
         * @property {boolean} [enhancedHostSupport] ServerData enhancedHostSupport
         * @property {number} [upvotePower] ServerData upvotePower
         * @property {Array.<string>} [connectEndPoints] ServerData connectEndPoints
         */

        /**
         * Constructs a new ServerData.
         * @memberof master
         * @classdesc Represents a ServerData.
         * @constructor
         * @param {master.IServerData=} [properties] Properties to set
         */
        function ServerData(properties) {
            this.resources = [];
            this.players = [];
            this.vars = {};
            this.connectEndPoints = [];
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ServerData svMaxclients.
         * @member {number}svMaxclients
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.svMaxclients = 0;

        /**
         * ServerData clients.
         * @member {number}clients
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.clients = 0;

        /**
         * ServerData protocol.
         * @member {number}protocol
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.protocol = 0;

        /**
         * ServerData hostname.
         * @member {string}hostname
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.hostname = "";

        /**
         * ServerData gametype.
         * @member {string}gametype
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.gametype = "";

        /**
         * ServerData mapname.
         * @member {string}mapname
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.mapname = "";

        /**
         * ServerData resources.
         * @member {Array.<string>}resources
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.resources = $util.emptyArray;

        /**
         * ServerData server.
         * @member {string}server
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.server = "";

        /**
         * ServerData players.
         * @member {Array.<master.IPlayer>}players
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.players = $util.emptyArray;

        /**
         * ServerData iconVersion.
         * @member {number}iconVersion
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.iconVersion = 0;

        /**
         * ServerData vars.
         * @member {Object.<string,string>}vars
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.vars = $util.emptyObject;

        /**
         * ServerData enhancedHostSupport.
         * @member {boolean}enhancedHostSupport
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.enhancedHostSupport = false;

        /**
         * ServerData upvotePower.
         * @member {number}upvotePower
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.upvotePower = 0;

        /**
         * ServerData connectEndPoints.
         * @member {Array.<string>}connectEndPoints
         * @memberof master.ServerData
         * @instance
         */
        ServerData.prototype.connectEndPoints = $util.emptyArray;

        /**
         * Creates a new ServerData instance using the specified properties.
         * @function create
         * @memberof master.ServerData
         * @static
         * @param {master.IServerData=} [properties] Properties to set
         * @returns {master.ServerData} ServerData instance
         */
        ServerData.create = function create(properties) {
            return new ServerData(properties);
        };

        /**
         * Encodes the specified ServerData message. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @function encode
         * @memberof master.ServerData
         * @static
         * @param {master.IServerData} message ServerData message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerData.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.svMaxclients != null && message.hasOwnProperty("svMaxclients"))
                writer.uint32(/* id 1, wireType 0 =*/8).int32(message.svMaxclients);
            if (message.clients != null && message.hasOwnProperty("clients"))
                writer.uint32(/* id 2, wireType 0 =*/16).int32(message.clients);
            if (message.protocol != null && message.hasOwnProperty("protocol"))
                writer.uint32(/* id 3, wireType 0 =*/24).int32(message.protocol);
            if (message.hostname != null && message.hasOwnProperty("hostname"))
                writer.uint32(/* id 4, wireType 2 =*/34).string(message.hostname);
            if (message.gametype != null && message.hasOwnProperty("gametype"))
                writer.uint32(/* id 5, wireType 2 =*/42).string(message.gametype);
            if (message.mapname != null && message.hasOwnProperty("mapname"))
                writer.uint32(/* id 6, wireType 2 =*/50).string(message.mapname);
            if (message.resources != null && message.resources.length)
                for (var i = 0; i < message.resources.length; ++i)
                    writer.uint32(/* id 8, wireType 2 =*/66).string(message.resources[i]);
            if (message.server != null && message.hasOwnProperty("server"))
                writer.uint32(/* id 9, wireType 2 =*/74).string(message.server);
            if (message.players != null && message.players.length)
                for (var i = 0; i < message.players.length; ++i)
                    $root.master.Player.encode(message.players[i], writer.uint32(/* id 10, wireType 2 =*/82).fork()).ldelim();
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                writer.uint32(/* id 11, wireType 0 =*/88).int32(message.iconVersion);
            if (message.vars != null && message.hasOwnProperty("vars"))
                for (var keys = Object.keys(message.vars), i = 0; i < keys.length; ++i)
                    writer.uint32(/* id 12, wireType 2 =*/98).fork().uint32(/* id 1, wireType 2 =*/10).string(keys[i]).uint32(/* id 2, wireType 2 =*/18).string(message.vars[keys[i]]).ldelim();
            if (message.enhancedHostSupport != null && message.hasOwnProperty("enhancedHostSupport"))
                writer.uint32(/* id 16, wireType 0 =*/128).bool(message.enhancedHostSupport);
            if (message.upvotePower != null && message.hasOwnProperty("upvotePower"))
                writer.uint32(/* id 17, wireType 0 =*/136).int32(message.upvotePower);
            if (message.connectEndPoints != null && message.connectEndPoints.length)
                for (var i = 0; i < message.connectEndPoints.length; ++i)
                    writer.uint32(/* id 18, wireType 2 =*/146).string(message.connectEndPoints[i]);
            return writer;
        };

        /**
         * Encodes the specified ServerData message, length delimited. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.ServerData
         * @static
         * @param {master.IServerData} message ServerData message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerData.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ServerData message from the specified reader or buffer.
         * @function decode
         * @memberof master.ServerData
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerData} ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerData.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.ServerData(), key;
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.svMaxclients = reader.int32();
                    break;
                case 2:
                    message.clients = reader.int32();
                    break;
                case 3:
                    message.protocol = reader.int32();
                    break;
                case 4:
                    message.hostname = reader.string();
                    break;
                case 5:
                    message.gametype = reader.string();
                    break;
                case 6:
                    message.mapname = reader.string();
                    break;
                case 8:
                    if (!(message.resources && message.resources.length))
                        message.resources = [];
                    message.resources.push(reader.string());
                    break;
                case 9:
                    message.server = reader.string();
                    break;
                case 10:
                    if (!(message.players && message.players.length))
                        message.players = [];
                    message.players.push($root.master.Player.decode(reader, reader.uint32()));
                    break;
                case 11:
                    message.iconVersion = reader.int32();
                    break;
                case 12:
                    reader.skip().pos++;
                    if (message.vars === $util.emptyObject)
                        message.vars = {};
                    key = reader.string();
                    reader.pos++;
                    message.vars[key] = reader.string();
                    break;
                case 16:
                    message.enhancedHostSupport = reader.bool();
                    break;
                case 17:
                    message.upvotePower = reader.int32();
                    break;
                case 18:
                    if (!(message.connectEndPoints && message.connectEndPoints.length))
                        message.connectEndPoints = [];
                    message.connectEndPoints.push(reader.string());
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ServerData message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.ServerData
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerData} ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerData.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ServerData message.
         * @function verify
         * @memberof master.ServerData
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ServerData.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.svMaxclients != null && message.hasOwnProperty("svMaxclients"))
                if (!$util.isInteger(message.svMaxclients))
                    return "svMaxclients: integer expected";
            if (message.clients != null && message.hasOwnProperty("clients"))
                if (!$util.isInteger(message.clients))
                    return "clients: integer expected";
            if (message.protocol != null && message.hasOwnProperty("protocol"))
                if (!$util.isInteger(message.protocol))
                    return "protocol: integer expected";
            if (message.hostname != null && message.hasOwnProperty("hostname"))
                if (!$util.isString(message.hostname))
                    return "hostname: string expected";
            if (message.gametype != null && message.hasOwnProperty("gametype"))
                if (!$util.isString(message.gametype))
                    return "gametype: string expected";
            if (message.mapname != null && message.hasOwnProperty("mapname"))
                if (!$util.isString(message.mapname))
                    return "mapname: string expected";
            if (message.resources != null && message.hasOwnProperty("resources")) {
                if (!Array.isArray(message.resources))
                    return "resources: array expected";
                for (var i = 0; i < message.resources.length; ++i)
                    if (!$util.isString(message.resources[i]))
                        return "resources: string[] expected";
            }
            if (message.server != null && message.hasOwnProperty("server"))
                if (!$util.isString(message.server))
                    return "server: string expected";
            if (message.players != null && message.hasOwnProperty("players")) {
                if (!Array.isArray(message.players))
                    return "players: array expected";
                for (var i = 0; i < message.players.length; ++i) {
                    var error = $root.master.Player.verify(message.players[i]);
                    if (error)
                        return "players." + error;
                }
            }
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                if (!$util.isInteger(message.iconVersion))
                    return "iconVersion: integer expected";
            if (message.vars != null && message.hasOwnProperty("vars")) {
                if (!$util.isObject(message.vars))
                    return "vars: object expected";
                var key = Object.keys(message.vars);
                for (var i = 0; i < key.length; ++i)
                    if (!$util.isString(message.vars[key[i]]))
                        return "vars: string{k:string} expected";
            }
            if (message.enhancedHostSupport != null && message.hasOwnProperty("enhancedHostSupport"))
                if (typeof message.enhancedHostSupport !== "boolean")
                    return "enhancedHostSupport: boolean expected";
            if (message.upvotePower != null && message.hasOwnProperty("upvotePower"))
                if (!$util.isInteger(message.upvotePower))
                    return "upvotePower: integer expected";
            if (message.connectEndPoints != null && message.hasOwnProperty("connectEndPoints")) {
                if (!Array.isArray(message.connectEndPoints))
                    return "connectEndPoints: array expected";
                for (var i = 0; i < message.connectEndPoints.length; ++i)
                    if (!$util.isString(message.connectEndPoints[i]))
                        return "connectEndPoints: string[] expected";
            }
            return null;
        };

        /**
         * Creates a ServerData message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.ServerData
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerData} ServerData
         */
        ServerData.fromObject = function fromObject(object) {
            if (object instanceof $root.master.ServerData)
                return object;
            var message = new $root.master.ServerData();
            if (object.svMaxclients != null)
                message.svMaxclients = object.svMaxclients | 0;
            if (object.clients != null)
                message.clients = object.clients | 0;
            if (object.protocol != null)
                message.protocol = object.protocol | 0;
            if (object.hostname != null)
                message.hostname = String(object.hostname);
            if (object.gametype != null)
                message.gametype = String(object.gametype);
            if (object.mapname != null)
                message.mapname = String(object.mapname);
            if (object.resources) {
                if (!Array.isArray(object.resources))
                    throw TypeError(".master.ServerData.resources: array expected");
                message.resources = [];
                for (var i = 0; i < object.resources.length; ++i)
                    message.resources[i] = String(object.resources[i]);
            }
            if (object.server != null)
                message.server = String(object.server);
            if (object.players) {
                if (!Array.isArray(object.players))
                    throw TypeError(".master.ServerData.players: array expected");
                message.players = [];
                for (var i = 0; i < object.players.length; ++i) {
                    if (typeof object.players[i] !== "object")
                        throw TypeError(".master.ServerData.players: object expected");
                    message.players[i] = $root.master.Player.fromObject(object.players[i]);
                }
            }
            if (object.iconVersion != null)
                message.iconVersion = object.iconVersion | 0;
            if (object.vars) {
                if (typeof object.vars !== "object")
                    throw TypeError(".master.ServerData.vars: object expected");
                message.vars = {};
                for (var keys = Object.keys(object.vars), i = 0; i < keys.length; ++i)
                    message.vars[keys[i]] = String(object.vars[keys[i]]);
            }
            if (object.enhancedHostSupport != null)
                message.enhancedHostSupport = Boolean(object.enhancedHostSupport);
            if (object.upvotePower != null)
                message.upvotePower = object.upvotePower | 0;
            if (object.connectEndPoints) {
                if (!Array.isArray(object.connectEndPoints))
                    throw TypeError(".master.ServerData.connectEndPoints: array expected");
                message.connectEndPoints = [];
                for (var i = 0; i < object.connectEndPoints.length; ++i)
                    message.connectEndPoints[i] = String(object.connectEndPoints[i]);
            }
            return message;
        };

        /**
         * Creates a plain object from a ServerData message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.ServerData
         * @static
         * @param {master.ServerData} message ServerData
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ServerData.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.arrays || options.defaults) {
                object.resources = [];
                object.players = [];
                object.connectEndPoints = [];
            }
            if (options.objects || options.defaults)
                object.vars = {};
            if (options.defaults) {
                object.svMaxclients = 0;
                object.clients = 0;
                object.protocol = 0;
                object.hostname = "";
                object.gametype = "";
                object.mapname = "";
                object.server = "";
                object.iconVersion = 0;
                object.enhancedHostSupport = false;
                object.upvotePower = 0;
            }
            if (message.svMaxclients != null && message.hasOwnProperty("svMaxclients"))
                object.svMaxclients = message.svMaxclients;
            if (message.clients != null && message.hasOwnProperty("clients"))
                object.clients = message.clients;
            if (message.protocol != null && message.hasOwnProperty("protocol"))
                object.protocol = message.protocol;
            if (message.hostname != null && message.hasOwnProperty("hostname"))
                object.hostname = message.hostname;
            if (message.gametype != null && message.hasOwnProperty("gametype"))
                object.gametype = message.gametype;
            if (message.mapname != null && message.hasOwnProperty("mapname"))
                object.mapname = message.mapname;
            if (message.resources && message.resources.length) {
                object.resources = [];
                for (var j = 0; j < message.resources.length; ++j)
                    object.resources[j] = message.resources[j];
            }
            if (message.server != null && message.hasOwnProperty("server"))
                object.server = message.server;
            if (message.players && message.players.length) {
                object.players = [];
                for (var j = 0; j < message.players.length; ++j)
                    object.players[j] = $root.master.Player.toObject(message.players[j], options);
            }
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                object.iconVersion = message.iconVersion;
            var keys2;
            if (message.vars && (keys2 = Object.keys(message.vars)).length) {
                object.vars = {};
                for (var j = 0; j < keys2.length; ++j)
                    object.vars[keys2[j]] = message.vars[keys2[j]];
            }
            if (message.enhancedHostSupport != null && message.hasOwnProperty("enhancedHostSupport"))
                object.enhancedHostSupport = message.enhancedHostSupport;
            if (message.upvotePower != null && message.hasOwnProperty("upvotePower"))
                object.upvotePower = message.upvotePower;
            if (message.connectEndPoints && message.connectEndPoints.length) {
                object.connectEndPoints = [];
                for (var j = 0; j < message.connectEndPoints.length; ++j)
                    object.connectEndPoints[j] = message.connectEndPoints[j];
            }
            return object;
        };

        /**
         * Converts this ServerData to JSON.
         * @function toJSON
         * @memberof master.ServerData
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ServerData.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return ServerData;
    })();

    master.Server = (function() {

        /**
         * Properties of a Server.
         * @memberof master
         * @interface IServer
         * @property {string} [EndPoint] Server EndPoint
         * @property {master.IServerData} [Data] Server Data
         */

        /**
         * Constructs a new Server.
         * @memberof master
         * @classdesc Represents a Server.
         * @constructor
         * @param {master.IServer=} [properties] Properties to set
         */
        function Server(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * Server EndPoint.
         * @member {string}EndPoint
         * @memberof master.Server
         * @instance
         */
        Server.prototype.EndPoint = "";

        /**
         * Server Data.
         * @member {(master.IServerData|null|undefined)}Data
         * @memberof master.Server
         * @instance
         */
        Server.prototype.Data = null;

        /**
         * Creates a new Server instance using the specified properties.
         * @function create
         * @memberof master.Server
         * @static
         * @param {master.IServer=} [properties] Properties to set
         * @returns {master.Server} Server instance
         */
        Server.create = function create(properties) {
            return new Server(properties);
        };

        /**
         * Encodes the specified Server message. Does not implicitly {@link master.Server.verify|verify} messages.
         * @function encode
         * @memberof master.Server
         * @static
         * @param {master.IServer} message Server message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Server.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.EndPoint != null && message.hasOwnProperty("EndPoint"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.EndPoint);
            if (message.Data != null && message.hasOwnProperty("Data"))
                $root.master.ServerData.encode(message.Data, writer.uint32(/* id 2, wireType 2 =*/18).fork()).ldelim();
            return writer;
        };

        /**
         * Encodes the specified Server message, length delimited. Does not implicitly {@link master.Server.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.Server
         * @static
         * @param {master.IServer} message Server message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Server.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a Server message from the specified reader or buffer.
         * @function decode
         * @memberof master.Server
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Server} Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Server.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.Server();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.EndPoint = reader.string();
                    break;
                case 2:
                    message.Data = $root.master.ServerData.decode(reader, reader.uint32());
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a Server message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.Server
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Server} Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Server.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a Server message.
         * @function verify
         * @memberof master.Server
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        Server.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.EndPoint != null && message.hasOwnProperty("EndPoint"))
                if (!$util.isString(message.EndPoint))
                    return "EndPoint: string expected";
            if (message.Data != null && message.hasOwnProperty("Data")) {
                var error = $root.master.ServerData.verify(message.Data);
                if (error)
                    return "Data." + error;
            }
            return null;
        };

        /**
         * Creates a Server message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.Server
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Server} Server
         */
        Server.fromObject = function fromObject(object) {
            if (object instanceof $root.master.Server)
                return object;
            var message = new $root.master.Server();
            if (object.EndPoint != null)
                message.EndPoint = String(object.EndPoint);
            if (object.Data != null) {
                if (typeof object.Data !== "object")
                    throw TypeError(".master.Server.Data: object expected");
                message.Data = $root.master.ServerData.fromObject(object.Data);
            }
            return message;
        };

        /**
         * Creates a plain object from a Server message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.Server
         * @static
         * @param {master.Server} message Server
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        Server.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.EndPoint = "";
                object.Data = null;
            }
            if (message.EndPoint != null && message.hasOwnProperty("EndPoint"))
                object.EndPoint = message.EndPoint;
            if (message.Data != null && message.hasOwnProperty("Data"))
                object.Data = $root.master.ServerData.toObject(message.Data, options);
            return object;
        };

        /**
         * Converts this Server to JSON.
         * @function toJSON
         * @memberof master.Server
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        Server.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return Server;
    })();

    master.Servers = (function() {

        /**
         * Properties of a Servers.
         * @memberof master
         * @interface IServers
         * @property {Array.<master.IServer>} [servers] Servers servers
         */

        /**
         * Constructs a new Servers.
         * @memberof master
         * @classdesc Represents a Servers.
         * @constructor
         * @param {master.IServers=} [properties] Properties to set
         */
        function Servers(properties) {
            this.servers = [];
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * Servers servers.
         * @member {Array.<master.IServer>}servers
         * @memberof master.Servers
         * @instance
         */
        Servers.prototype.servers = $util.emptyArray;

        /**
         * Creates a new Servers instance using the specified properties.
         * @function create
         * @memberof master.Servers
         * @static
         * @param {master.IServers=} [properties] Properties to set
         * @returns {master.Servers} Servers instance
         */
        Servers.create = function create(properties) {
            return new Servers(properties);
        };

        /**
         * Encodes the specified Servers message. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @function encode
         * @memberof master.Servers
         * @static
         * @param {master.IServers} message Servers message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Servers.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.servers != null && message.servers.length)
                for (var i = 0; i < message.servers.length; ++i)
                    $root.master.Server.encode(message.servers[i], writer.uint32(/* id 1, wireType 2 =*/10).fork()).ldelim();
            return writer;
        };

        /**
         * Encodes the specified Servers message, length delimited. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.Servers
         * @static
         * @param {master.IServers} message Servers message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        Servers.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a Servers message from the specified reader or buffer.
         * @function decode
         * @memberof master.Servers
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Servers} Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Servers.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.Servers();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    if (!(message.servers && message.servers.length))
                        message.servers = [];
                    message.servers.push($root.master.Server.decode(reader, reader.uint32()));
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a Servers message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.Servers
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Servers} Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        Servers.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a Servers message.
         * @function verify
         * @memberof master.Servers
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        Servers.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.servers != null && message.hasOwnProperty("servers")) {
                if (!Array.isArray(message.servers))
                    return "servers: array expected";
                for (var i = 0; i < message.servers.length; ++i) {
                    var error = $root.master.Server.verify(message.servers[i]);
                    if (error)
                        return "servers." + error;
                }
            }
            return null;
        };

        /**
         * Creates a Servers message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.Servers
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Servers} Servers
         */
        Servers.fromObject = function fromObject(object) {
            if (object instanceof $root.master.Servers)
                return object;
            var message = new $root.master.Servers();
            if (object.servers) {
                if (!Array.isArray(object.servers))
                    throw TypeError(".master.Servers.servers: array expected");
                message.servers = [];
                for (var i = 0; i < object.servers.length; ++i) {
                    if (typeof object.servers[i] !== "object")
                        throw TypeError(".master.Servers.servers: object expected");
                    message.servers[i] = $root.master.Server.fromObject(object.servers[i]);
                }
            }
            return message;
        };

        /**
         * Creates a plain object from a Servers message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.Servers
         * @static
         * @param {master.Servers} message Servers
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        Servers.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.arrays || options.defaults)
                object.servers = [];
            if (message.servers && message.servers.length) {
                object.servers = [];
                for (var j = 0; j < message.servers.length; ++j)
                    object.servers[j] = $root.master.Server.toObject(message.servers[j], options);
            }
            return object;
        };

        /**
         * Converts this Servers to JSON.
         * @function toJSON
         * @memberof master.Servers
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        Servers.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return Servers;
    })();

    master.ServerIcon = (function() {

        /**
         * Properties of a ServerIcon.
         * @memberof master
         * @interface IServerIcon
         * @property {string} [endPoint] ServerIcon endPoint
         * @property {Uint8Array} [icon] ServerIcon icon
         * @property {number} [iconVersion] ServerIcon iconVersion
         */

        /**
         * Constructs a new ServerIcon.
         * @memberof master
         * @classdesc Represents a ServerIcon.
         * @constructor
         * @param {master.IServerIcon=} [properties] Properties to set
         */
        function ServerIcon(properties) {
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ServerIcon endPoint.
         * @member {string}endPoint
         * @memberof master.ServerIcon
         * @instance
         */
        ServerIcon.prototype.endPoint = "";

        /**
         * ServerIcon icon.
         * @member {Uint8Array}icon
         * @memberof master.ServerIcon
         * @instance
         */
        ServerIcon.prototype.icon = $util.newBuffer([]);

        /**
         * ServerIcon iconVersion.
         * @member {number}iconVersion
         * @memberof master.ServerIcon
         * @instance
         */
        ServerIcon.prototype.iconVersion = 0;

        /**
         * Creates a new ServerIcon instance using the specified properties.
         * @function create
         * @memberof master.ServerIcon
         * @static
         * @param {master.IServerIcon=} [properties] Properties to set
         * @returns {master.ServerIcon} ServerIcon instance
         */
        ServerIcon.create = function create(properties) {
            return new ServerIcon(properties);
        };

        /**
         * Encodes the specified ServerIcon message. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @function encode
         * @memberof master.ServerIcon
         * @static
         * @param {master.IServerIcon} message ServerIcon message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerIcon.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.endPoint != null && message.hasOwnProperty("endPoint"))
                writer.uint32(/* id 1, wireType 2 =*/10).string(message.endPoint);
            if (message.icon != null && message.hasOwnProperty("icon"))
                writer.uint32(/* id 2, wireType 2 =*/18).bytes(message.icon);
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                writer.uint32(/* id 3, wireType 0 =*/24).int32(message.iconVersion);
            return writer;
        };

        /**
         * Encodes the specified ServerIcon message, length delimited. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.ServerIcon
         * @static
         * @param {master.IServerIcon} message ServerIcon message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerIcon.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ServerIcon message from the specified reader or buffer.
         * @function decode
         * @memberof master.ServerIcon
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerIcon} ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerIcon.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.ServerIcon();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    message.endPoint = reader.string();
                    break;
                case 2:
                    message.icon = reader.bytes();
                    break;
                case 3:
                    message.iconVersion = reader.int32();
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ServerIcon message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.ServerIcon
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerIcon} ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerIcon.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ServerIcon message.
         * @function verify
         * @memberof master.ServerIcon
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ServerIcon.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.endPoint != null && message.hasOwnProperty("endPoint"))
                if (!$util.isString(message.endPoint))
                    return "endPoint: string expected";
            if (message.icon != null && message.hasOwnProperty("icon"))
                if (!(message.icon && typeof message.icon.length === "number" || $util.isString(message.icon)))
                    return "icon: buffer expected";
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                if (!$util.isInteger(message.iconVersion))
                    return "iconVersion: integer expected";
            return null;
        };

        /**
         * Creates a ServerIcon message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.ServerIcon
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcon} ServerIcon
         */
        ServerIcon.fromObject = function fromObject(object) {
            if (object instanceof $root.master.ServerIcon)
                return object;
            var message = new $root.master.ServerIcon();
            if (object.endPoint != null)
                message.endPoint = String(object.endPoint);
            if (object.icon != null)
                if (typeof object.icon === "string")
                    $util.base64.decode(object.icon, message.icon = $util.newBuffer($util.base64.length(object.icon)), 0);
                else if (object.icon.length)
                    message.icon = object.icon;
            if (object.iconVersion != null)
                message.iconVersion = object.iconVersion | 0;
            return message;
        };

        /**
         * Creates a plain object from a ServerIcon message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.ServerIcon
         * @static
         * @param {master.ServerIcon} message ServerIcon
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ServerIcon.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.defaults) {
                object.endPoint = "";
                object.icon = options.bytes === String ? "" : [];
                object.iconVersion = 0;
            }
            if (message.endPoint != null && message.hasOwnProperty("endPoint"))
                object.endPoint = message.endPoint;
            if (message.icon != null && message.hasOwnProperty("icon"))
                object.icon = options.bytes === String ? $util.base64.encode(message.icon, 0, message.icon.length) : options.bytes === Array ? Array.prototype.slice.call(message.icon) : message.icon;
            if (message.iconVersion != null && message.hasOwnProperty("iconVersion"))
                object.iconVersion = message.iconVersion;
            return object;
        };

        /**
         * Converts this ServerIcon to JSON.
         * @function toJSON
         * @memberof master.ServerIcon
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ServerIcon.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return ServerIcon;
    })();

    master.ServerIcons = (function() {

        /**
         * Properties of a ServerIcons.
         * @memberof master
         * @interface IServerIcons
         * @property {Array.<master.IServerIcon>} [icons] ServerIcons icons
         */

        /**
         * Constructs a new ServerIcons.
         * @memberof master
         * @classdesc Represents a ServerIcons.
         * @constructor
         * @param {master.IServerIcons=} [properties] Properties to set
         */
        function ServerIcons(properties) {
            this.icons = [];
            if (properties)
                for (var keys = Object.keys(properties), i = 0; i < keys.length; ++i)
                    if (properties[keys[i]] != null)
                        this[keys[i]] = properties[keys[i]];
        }

        /**
         * ServerIcons icons.
         * @member {Array.<master.IServerIcon>}icons
         * @memberof master.ServerIcons
         * @instance
         */
        ServerIcons.prototype.icons = $util.emptyArray;

        /**
         * Creates a new ServerIcons instance using the specified properties.
         * @function create
         * @memberof master.ServerIcons
         * @static
         * @param {master.IServerIcons=} [properties] Properties to set
         * @returns {master.ServerIcons} ServerIcons instance
         */
        ServerIcons.create = function create(properties) {
            return new ServerIcons(properties);
        };

        /**
         * Encodes the specified ServerIcons message. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @function encode
         * @memberof master.ServerIcons
         * @static
         * @param {master.IServerIcons} message ServerIcons message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerIcons.encode = function encode(message, writer) {
            if (!writer)
                writer = $Writer.create();
            if (message.icons != null && message.icons.length)
                for (var i = 0; i < message.icons.length; ++i)
                    $root.master.ServerIcon.encode(message.icons[i], writer.uint32(/* id 1, wireType 2 =*/10).fork()).ldelim();
            return writer;
        };

        /**
         * Encodes the specified ServerIcons message, length delimited. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @function encodeDelimited
         * @memberof master.ServerIcons
         * @static
         * @param {master.IServerIcons} message ServerIcons message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        ServerIcons.encodeDelimited = function encodeDelimited(message, writer) {
            return this.encode(message, writer).ldelim();
        };

        /**
         * Decodes a ServerIcons message from the specified reader or buffer.
         * @function decode
         * @memberof master.ServerIcons
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerIcons} ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerIcons.decode = function decode(reader, length) {
            if (!(reader instanceof $Reader))
                reader = $Reader.create(reader);
            var end = length === undefined ? reader.len : reader.pos + length, message = new $root.master.ServerIcons();
            while (reader.pos < end) {
                var tag = reader.uint32();
                switch (tag >>> 3) {
                case 1:
                    if (!(message.icons && message.icons.length))
                        message.icons = [];
                    message.icons.push($root.master.ServerIcon.decode(reader, reader.uint32()));
                    break;
                default:
                    reader.skipType(tag & 7);
                    break;
                }
            }
            return message;
        };

        /**
         * Decodes a ServerIcons message from the specified reader or buffer, length delimited.
         * @function decodeDelimited
         * @memberof master.ServerIcons
         * @static
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerIcons} ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        ServerIcons.decodeDelimited = function decodeDelimited(reader) {
            if (!(reader instanceof $Reader))
                reader = new $Reader(reader);
            return this.decode(reader, reader.uint32());
        };

        /**
         * Verifies a ServerIcons message.
         * @function verify
         * @memberof master.ServerIcons
         * @static
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {string|null} `null` if valid, otherwise the reason why it is not
         */
        ServerIcons.verify = function verify(message) {
            if (typeof message !== "object" || message === null)
                return "object expected";
            if (message.icons != null && message.hasOwnProperty("icons")) {
                if (!Array.isArray(message.icons))
                    return "icons: array expected";
                for (var i = 0; i < message.icons.length; ++i) {
                    var error = $root.master.ServerIcon.verify(message.icons[i]);
                    if (error)
                        return "icons." + error;
                }
            }
            return null;
        };

        /**
         * Creates a ServerIcons message from a plain object. Also converts values to their respective internal types.
         * @function fromObject
         * @memberof master.ServerIcons
         * @static
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcons} ServerIcons
         */
        ServerIcons.fromObject = function fromObject(object) {
            if (object instanceof $root.master.ServerIcons)
                return object;
            var message = new $root.master.ServerIcons();
            if (object.icons) {
                if (!Array.isArray(object.icons))
                    throw TypeError(".master.ServerIcons.icons: array expected");
                message.icons = [];
                for (var i = 0; i < object.icons.length; ++i) {
                    if (typeof object.icons[i] !== "object")
                        throw TypeError(".master.ServerIcons.icons: object expected");
                    message.icons[i] = $root.master.ServerIcon.fromObject(object.icons[i]);
                }
            }
            return message;
        };

        /**
         * Creates a plain object from a ServerIcons message. Also converts values to other types if specified.
         * @function toObject
         * @memberof master.ServerIcons
         * @static
         * @param {master.ServerIcons} message ServerIcons
         * @param {$protobuf.IConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        ServerIcons.toObject = function toObject(message, options) {
            if (!options)
                options = {};
            var object = {};
            if (options.arrays || options.defaults)
                object.icons = [];
            if (message.icons && message.icons.length) {
                object.icons = [];
                for (var j = 0; j < message.icons.length; ++j)
                    object.icons[j] = $root.master.ServerIcon.toObject(message.icons[j], options);
            }
            return object;
        };

        /**
         * Converts this ServerIcons to JSON.
         * @function toJSON
         * @memberof master.ServerIcons
         * @instance
         * @returns {Object.<string,*>} JSON object
         */
        ServerIcons.prototype.toJSON = function toJSON() {
            return this.constructor.toObject(this, $protobuf.util.toJSONOptions);
        };

        return ServerIcons;
    })();

    return master;
})();

module.exports = $root;
