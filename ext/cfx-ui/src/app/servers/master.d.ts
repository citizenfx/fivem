import * as $protobuf from "protobufjs";

/**
 * Namespace master.
 * @exports master
 * @namespace
 */
export namespace master {

    type Player$Properties = {
        name?: string;
        identifiers?: string[];
        endpoint?: string;
        ping?: number;
        id?: number;
    };

    /**
     * Constructs a new Player.
     * @exports master.Player
     * @constructor
     * @param {master.Player$Properties=} [properties] Properties to set
     */
    class Player {

        /**
         * Constructs a new Player.
         * @exports master.Player
         * @constructor
         * @param {master.Player$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.Player$Properties);

        /**
         * Player name.
         * @type {string}
         */
        public name: string;

        /**
         * Player identifiers.
         * @type {Array.<string>}
         */
        public identifiers: string[];

        /**
         * Player endpoint.
         * @type {string}
         */
        public endpoint: string;

        /**
         * Player ping.
         * @type {number}
         */
        public ping: number;

        /**
         * Player id.
         * @type {number}
         */
        public id: number;

        /**
         * Creates a new Player instance using the specified properties.
         * @param {master.Player$Properties=} [properties] Properties to set
         * @returns {master.Player} Player instance
         */
        public static create(properties?: master.Player$Properties): master.Player;

        /**
         * Encodes the specified Player message. Does not implicitly {@link master.Player.verify|verify} messages.
         * @param {master.Player$Properties} message Player message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.Player$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Player message, length delimited. Does not implicitly {@link master.Player.verify|verify} messages.
         * @param {master.Player$Properties} message Player message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.Player$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Player message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Player} Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Player;

        /**
         * Decodes a Player message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Player} Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Player;

        /**
         * Verifies a Player message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a Player message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Player} Player
         */
        public static fromObject(object: { [k: string]: any }): master.Player;

        /**
         * Creates a Player message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.Player.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Player} Player
         */
        public static from(object: { [k: string]: any }): master.Player;

        /**
         * Creates a plain object from a Player message. Also converts values to other types if specified.
         * @param {master.Player} message Player
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.Player, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this Player message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this Player to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    type ServerData$Properties = {
        svMaxclients?: number;
        clients?: number;
        protocol?: number;
        hostname?: string;
        gametype?: string;
        mapname?: string;
        resources?: string[];
        server?: string;
        players?: master.Player$Properties[];
        iconVersion?: number;
        enhancedHostSupport?: boolean;
    };

    /**
     * Constructs a new ServerData.
     * @exports master.ServerData
     * @constructor
     * @param {master.ServerData$Properties=} [properties] Properties to set
     */
    class ServerData {

        /**
         * Constructs a new ServerData.
         * @exports master.ServerData
         * @constructor
         * @param {master.ServerData$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.ServerData$Properties);

        /**
         * ServerData svMaxclients.
         * @type {number}
         */
        public svMaxclients: number;

        /**
         * ServerData clients.
         * @type {number}
         */
        public clients: number;

        /**
         * ServerData protocol.
         * @type {number}
         */
        public protocol: number;

        /**
         * ServerData hostname.
         * @type {string}
         */
        public hostname: string;

        /**
         * ServerData gametype.
         * @type {string}
         */
        public gametype: string;

        /**
         * ServerData mapname.
         * @type {string}
         */
        public mapname: string;

        /**
         * ServerData resources.
         * @type {Array.<string>}
         */
        public resources: string[];

        /**
         * ServerData server.
         * @type {string}
         */
        public server: string;

        /**
         * ServerData players.
         * @type {Array.<master.Player$Properties>}
         */
        public players: master.Player$Properties[];

        /**
         * ServerData iconVersion.
         * @type {number}
         */
        public iconVersion: number;

        /**
         * ServerData enhancedHostSupport.
         * @type {boolean}
         */
        public enhancedHostSupport: boolean;

        /**
         * Creates a new ServerData instance using the specified properties.
         * @param {master.ServerData$Properties=} [properties] Properties to set
         * @returns {master.ServerData} ServerData instance
         */
        public static create(properties?: master.ServerData$Properties): master.ServerData;

        /**
         * Encodes the specified ServerData message. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @param {master.ServerData$Properties} message ServerData message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.ServerData$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerData message, length delimited. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @param {master.ServerData$Properties} message ServerData message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.ServerData$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerData message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerData} ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerData;

        /**
         * Decodes a ServerData message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerData} ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerData;

        /**
         * Verifies a ServerData message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a ServerData message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerData} ServerData
         */
        public static fromObject(object: { [k: string]: any }): master.ServerData;

        /**
         * Creates a ServerData message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.ServerData.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerData} ServerData
         */
        public static from(object: { [k: string]: any }): master.ServerData;

        /**
         * Creates a plain object from a ServerData message. Also converts values to other types if specified.
         * @param {master.ServerData} message ServerData
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.ServerData, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this ServerData message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerData to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    type Server$Properties = {
        EndPoint?: string;
        Data?: master.ServerData$Properties;
    };

    /**
     * Constructs a new Server.
     * @exports master.Server
     * @constructor
     * @param {master.Server$Properties=} [properties] Properties to set
     */
    class Server {

        /**
         * Constructs a new Server.
         * @exports master.Server
         * @constructor
         * @param {master.Server$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.Server$Properties);

        /**
         * Server EndPoint.
         * @type {string}
         */
        public EndPoint: string;

        /**
         * Server Data.
         * @type {(master.ServerData$Properties|null)}
         */
        public Data: (master.ServerData$Properties|null);

        /**
         * Creates a new Server instance using the specified properties.
         * @param {master.Server$Properties=} [properties] Properties to set
         * @returns {master.Server} Server instance
         */
        public static create(properties?: master.Server$Properties): master.Server;

        /**
         * Encodes the specified Server message. Does not implicitly {@link master.Server.verify|verify} messages.
         * @param {master.Server$Properties} message Server message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.Server$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Server message, length delimited. Does not implicitly {@link master.Server.verify|verify} messages.
         * @param {master.Server$Properties} message Server message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.Server$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Server message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Server} Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Server;

        /**
         * Decodes a Server message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Server} Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Server;

        /**
         * Verifies a Server message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a Server message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Server} Server
         */
        public static fromObject(object: { [k: string]: any }): master.Server;

        /**
         * Creates a Server message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.Server.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Server} Server
         */
        public static from(object: { [k: string]: any }): master.Server;

        /**
         * Creates a plain object from a Server message. Also converts values to other types if specified.
         * @param {master.Server} message Server
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.Server, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this Server message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this Server to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    type Servers$Properties = {
        servers?: master.Server$Properties[];
    };

    /**
     * Constructs a new Servers.
     * @exports master.Servers
     * @constructor
     * @param {master.Servers$Properties=} [properties] Properties to set
     */
    class Servers {

        /**
         * Constructs a new Servers.
         * @exports master.Servers
         * @constructor
         * @param {master.Servers$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.Servers$Properties);

        /**
         * Servers servers.
         * @type {Array.<master.Server$Properties>}
         */
        public servers: master.Server$Properties[];

        /**
         * Creates a new Servers instance using the specified properties.
         * @param {master.Servers$Properties=} [properties] Properties to set
         * @returns {master.Servers} Servers instance
         */
        public static create(properties?: master.Servers$Properties): master.Servers;

        /**
         * Encodes the specified Servers message. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @param {master.Servers$Properties} message Servers message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.Servers$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Servers message, length delimited. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @param {master.Servers$Properties} message Servers message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.Servers$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Servers message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.Servers} Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Servers;

        /**
         * Decodes a Servers message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.Servers} Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Servers;

        /**
         * Verifies a Servers message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a Servers message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Servers} Servers
         */
        public static fromObject(object: { [k: string]: any }): master.Servers;

        /**
         * Creates a Servers message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.Servers.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.Servers} Servers
         */
        public static from(object: { [k: string]: any }): master.Servers;

        /**
         * Creates a plain object from a Servers message. Also converts values to other types if specified.
         * @param {master.Servers} message Servers
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.Servers, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this Servers message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this Servers to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    type ServerIcon$Properties = {
        endPoint?: string;
        icon?: Uint8Array;
        iconVersion?: number;
    };

    /**
     * Constructs a new ServerIcon.
     * @exports master.ServerIcon
     * @constructor
     * @param {master.ServerIcon$Properties=} [properties] Properties to set
     */
    class ServerIcon {

        /**
         * Constructs a new ServerIcon.
         * @exports master.ServerIcon
         * @constructor
         * @param {master.ServerIcon$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.ServerIcon$Properties);

        /**
         * ServerIcon endPoint.
         * @type {string}
         */
        public endPoint: string;

        /**
         * ServerIcon icon.
         * @type {Uint8Array}
         */
        public icon: Uint8Array;

        /**
         * ServerIcon iconVersion.
         * @type {number}
         */
        public iconVersion: number;

        /**
         * Creates a new ServerIcon instance using the specified properties.
         * @param {master.ServerIcon$Properties=} [properties] Properties to set
         * @returns {master.ServerIcon} ServerIcon instance
         */
        public static create(properties?: master.ServerIcon$Properties): master.ServerIcon;

        /**
         * Encodes the specified ServerIcon message. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @param {master.ServerIcon$Properties} message ServerIcon message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.ServerIcon$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerIcon message, length delimited. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @param {master.ServerIcon$Properties} message ServerIcon message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.ServerIcon$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerIcon message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerIcon} ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerIcon;

        /**
         * Decodes a ServerIcon message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerIcon} ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerIcon;

        /**
         * Verifies a ServerIcon message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a ServerIcon message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcon} ServerIcon
         */
        public static fromObject(object: { [k: string]: any }): master.ServerIcon;

        /**
         * Creates a ServerIcon message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.ServerIcon.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcon} ServerIcon
         */
        public static from(object: { [k: string]: any }): master.ServerIcon;

        /**
         * Creates a plain object from a ServerIcon message. Also converts values to other types if specified.
         * @param {master.ServerIcon} message ServerIcon
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.ServerIcon, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this ServerIcon message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerIcon to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    type ServerIcons$Properties = {
        icons?: master.ServerIcon$Properties[];
    };

    /**
     * Constructs a new ServerIcons.
     * @exports master.ServerIcons
     * @constructor
     * @param {master.ServerIcons$Properties=} [properties] Properties to set
     */
    class ServerIcons {

        /**
         * Constructs a new ServerIcons.
         * @exports master.ServerIcons
         * @constructor
         * @param {master.ServerIcons$Properties=} [properties] Properties to set
         */
        constructor(properties?: master.ServerIcons$Properties);

        /**
         * ServerIcons icons.
         * @type {Array.<master.ServerIcon$Properties>}
         */
        public icons: master.ServerIcon$Properties[];

        /**
         * Creates a new ServerIcons instance using the specified properties.
         * @param {master.ServerIcons$Properties=} [properties] Properties to set
         * @returns {master.ServerIcons} ServerIcons instance
         */
        public static create(properties?: master.ServerIcons$Properties): master.ServerIcons;

        /**
         * Encodes the specified ServerIcons message. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @param {master.ServerIcons$Properties} message ServerIcons message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encode(message: master.ServerIcons$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerIcons message, length delimited. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @param {master.ServerIcons$Properties} message ServerIcons message or plain object to encode
         * @param {$protobuf.Writer} [writer] Writer to encode to
         * @returns {$protobuf.Writer} Writer
         */
        public static encodeDelimited(message: master.ServerIcons$Properties, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerIcons message from the specified reader or buffer.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @param {number} [length] Message length if known beforehand
         * @returns {master.ServerIcons} ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerIcons;

        /**
         * Decodes a ServerIcons message from the specified reader or buffer, length delimited.
         * @param {$protobuf.Reader|Uint8Array} reader Reader or buffer to decode from
         * @returns {master.ServerIcons} ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerIcons;

        /**
         * Verifies a ServerIcons message.
         * @param {Object.<string,*>} message Plain object to verify
         * @returns {?string} `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): string;

        /**
         * Creates a ServerIcons message from a plain object. Also converts values to their respective internal types.
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcons} ServerIcons
         */
        public static fromObject(object: { [k: string]: any }): master.ServerIcons;

        /**
         * Creates a ServerIcons message from a plain object. Also converts values to their respective internal types.
         * This is an alias of {@link master.ServerIcons.fromObject}.
         * @function
         * @param {Object.<string,*>} object Plain object
         * @returns {master.ServerIcons} ServerIcons
         */
        public static from(object: { [k: string]: any }): master.ServerIcons;

        /**
         * Creates a plain object from a ServerIcons message. Also converts values to other types if specified.
         * @param {master.ServerIcons} message ServerIcons
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public static toObject(message: master.ServerIcons, options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Creates a plain object from this ServerIcons message. Also converts values to other types if specified.
         * @param {$protobuf.ConversionOptions} [options] Conversion options
         * @returns {Object.<string,*>} Plain object
         */
        public toObject(options?: $protobuf.ConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerIcons to JSON.
         * @returns {Object.<string,*>} JSON object
         */
        public toJSON(): { [k: string]: any };
    }
}
