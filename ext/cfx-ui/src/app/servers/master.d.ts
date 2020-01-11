import * as $protobuf from "protobufjs";

/** Namespace master. */
export namespace master {

    /** Properties of a Player. */
    interface IPlayer {

        /** Player name */
        name?: string;

        /** Player identifiers */
        identifiers?: string[];

        /** Player endpoint */
        endpoint?: string;

        /** Player ping */
        ping?: number;

        /** Player id */
        id?: number;
    }

    /** Represents a Player. */
    class Player {

        /**
         * Constructs a new Player.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IPlayer);

        /** Player name. */
        public name: string;

        /** Player identifiers. */
        public identifiers: string[];

        /** Player endpoint. */
        public endpoint: string;

        /** Player ping. */
        public ping: number;

        /** Player id. */
        public id: number;

        /**
         * Creates a new Player instance using the specified properties.
         * @param [properties] Properties to set
         * @returns Player instance
         */
        public static create(properties?: master.IPlayer): master.Player;

        /**
         * Encodes the specified Player message. Does not implicitly {@link master.Player.verify|verify} messages.
         * @param message Player message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IPlayer, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Player message, length delimited. Does not implicitly {@link master.Player.verify|verify} messages.
         * @param message Player message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IPlayer, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Player message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Player;

        /**
         * Decodes a Player message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns Player
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Player;

        /**
         * Verifies a Player message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a Player message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns Player
         */
        public static fromObject(object: { [k: string]: any }): master.Player;

        /**
         * Creates a plain object from a Player message. Also converts values to other types if specified.
         * @param message Player
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.Player, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this Player to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    /** Properties of a ServerData. */
    interface IServerData {

        /** ServerData svMaxclients */
        svMaxclients?: number;

        /** ServerData clients */
        clients?: number;

        /** ServerData protocol */
        protocol?: number;

        /** ServerData hostname */
        hostname?: string;

        /** ServerData gametype */
        gametype?: string;

        /** ServerData mapname */
        mapname?: string;

        /** ServerData resources */
        resources?: string[];

        /** ServerData server */
        server?: string;

        /** ServerData players */
        players?: master.IPlayer[];

        /** ServerData iconVersion */
        iconVersion?: number;

        /** ServerData vars */
        vars?: { [k: string]: string };

        /** ServerData enhancedHostSupport */
        enhancedHostSupport?: boolean;

        /** ServerData upvotePower */
        upvotePower?: number;

        /** ServerData connectEndPoints */
        connectEndPoints?: string[];
    }

    /** Represents a ServerData. */
    class ServerData {

        /**
         * Constructs a new ServerData.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IServerData);

        /** ServerData svMaxclients. */
        public svMaxclients: number;

        /** ServerData clients. */
        public clients: number;

        /** ServerData protocol. */
        public protocol: number;

        /** ServerData hostname. */
        public hostname: string;

        /** ServerData gametype. */
        public gametype: string;

        /** ServerData mapname. */
        public mapname: string;

        /** ServerData resources. */
        public resources: string[];

        /** ServerData server. */
        public server: string;

        /** ServerData players. */
        public players: master.IPlayer[];

        /** ServerData iconVersion. */
        public iconVersion: number;

        /** ServerData vars. */
        public vars: { [k: string]: string };

        /** ServerData enhancedHostSupport. */
        public enhancedHostSupport: boolean;

        /** ServerData upvotePower. */
        public upvotePower: number;

        /** ServerData connectEndPoints. */
        public connectEndPoints: string[];

        /**
         * Creates a new ServerData instance using the specified properties.
         * @param [properties] Properties to set
         * @returns ServerData instance
         */
        public static create(properties?: master.IServerData): master.ServerData;

        /**
         * Encodes the specified ServerData message. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @param message ServerData message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IServerData, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerData message, length delimited. Does not implicitly {@link master.ServerData.verify|verify} messages.
         * @param message ServerData message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IServerData, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerData message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerData;

        /**
         * Decodes a ServerData message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns ServerData
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerData;

        /**
         * Verifies a ServerData message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a ServerData message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns ServerData
         */
        public static fromObject(object: { [k: string]: any }): master.ServerData;

        /**
         * Creates a plain object from a ServerData message. Also converts values to other types if specified.
         * @param message ServerData
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.ServerData, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerData to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    /** Properties of a Server. */
    interface IServer {

        /** Server EndPoint */
        EndPoint?: string;

        /** Server Data */
        Data?: master.IServerData;
    }

    /** Represents a Server. */
    class Server {

        /**
         * Constructs a new Server.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IServer);

        /** Server EndPoint. */
        public EndPoint: string;

        /** Server Data. */
        public Data?: (master.IServerData|null);

        /**
         * Creates a new Server instance using the specified properties.
         * @param [properties] Properties to set
         * @returns Server instance
         */
        public static create(properties?: master.IServer): master.Server;

        /**
         * Encodes the specified Server message. Does not implicitly {@link master.Server.verify|verify} messages.
         * @param message Server message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IServer, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Server message, length delimited. Does not implicitly {@link master.Server.verify|verify} messages.
         * @param message Server message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IServer, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Server message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Server;

        /**
         * Decodes a Server message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns Server
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Server;

        /**
         * Verifies a Server message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a Server message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns Server
         */
        public static fromObject(object: { [k: string]: any }): master.Server;

        /**
         * Creates a plain object from a Server message. Also converts values to other types if specified.
         * @param message Server
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.Server, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this Server to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    /** Properties of a Servers. */
    interface IServers {

        /** Servers servers */
        servers?: master.IServer[];
    }

    /** Represents a Servers. */
    class Servers {

        /**
         * Constructs a new Servers.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IServers);

        /** Servers servers. */
        public servers: master.IServer[];

        /**
         * Creates a new Servers instance using the specified properties.
         * @param [properties] Properties to set
         * @returns Servers instance
         */
        public static create(properties?: master.IServers): master.Servers;

        /**
         * Encodes the specified Servers message. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @param message Servers message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IServers, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified Servers message, length delimited. Does not implicitly {@link master.Servers.verify|verify} messages.
         * @param message Servers message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IServers, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a Servers message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.Servers;

        /**
         * Decodes a Servers message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns Servers
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.Servers;

        /**
         * Verifies a Servers message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a Servers message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns Servers
         */
        public static fromObject(object: { [k: string]: any }): master.Servers;

        /**
         * Creates a plain object from a Servers message. Also converts values to other types if specified.
         * @param message Servers
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.Servers, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this Servers to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    /** Properties of a ServerIcon. */
    interface IServerIcon {

        /** ServerIcon endPoint */
        endPoint?: string;

        /** ServerIcon icon */
        icon?: Uint8Array;

        /** ServerIcon iconVersion */
        iconVersion?: number;
    }

    /** Represents a ServerIcon. */
    class ServerIcon {

        /**
         * Constructs a new ServerIcon.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IServerIcon);

        /** ServerIcon endPoint. */
        public endPoint: string;

        /** ServerIcon icon. */
        public icon: Uint8Array;

        /** ServerIcon iconVersion. */
        public iconVersion: number;

        /**
         * Creates a new ServerIcon instance using the specified properties.
         * @param [properties] Properties to set
         * @returns ServerIcon instance
         */
        public static create(properties?: master.IServerIcon): master.ServerIcon;

        /**
         * Encodes the specified ServerIcon message. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @param message ServerIcon message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IServerIcon, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerIcon message, length delimited. Does not implicitly {@link master.ServerIcon.verify|verify} messages.
         * @param message ServerIcon message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IServerIcon, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerIcon message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerIcon;

        /**
         * Decodes a ServerIcon message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns ServerIcon
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerIcon;

        /**
         * Verifies a ServerIcon message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a ServerIcon message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns ServerIcon
         */
        public static fromObject(object: { [k: string]: any }): master.ServerIcon;

        /**
         * Creates a plain object from a ServerIcon message. Also converts values to other types if specified.
         * @param message ServerIcon
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.ServerIcon, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerIcon to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }

    /** Properties of a ServerIcons. */
    interface IServerIcons {

        /** ServerIcons icons */
        icons?: master.IServerIcon[];
    }

    /** Represents a ServerIcons. */
    class ServerIcons {

        /**
         * Constructs a new ServerIcons.
         * @param [properties] Properties to set
         */
        constructor(properties?: master.IServerIcons);

        /** ServerIcons icons. */
        public icons: master.IServerIcon[];

        /**
         * Creates a new ServerIcons instance using the specified properties.
         * @param [properties] Properties to set
         * @returns ServerIcons instance
         */
        public static create(properties?: master.IServerIcons): master.ServerIcons;

        /**
         * Encodes the specified ServerIcons message. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @param message ServerIcons message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encode(message: master.IServerIcons, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Encodes the specified ServerIcons message, length delimited. Does not implicitly {@link master.ServerIcons.verify|verify} messages.
         * @param message ServerIcons message or plain object to encode
         * @param [writer] Writer to encode to
         * @returns Writer
         */
        public static encodeDelimited(message: master.IServerIcons, writer?: $protobuf.Writer): $protobuf.Writer;

        /**
         * Decodes a ServerIcons message from the specified reader or buffer.
         * @param reader Reader or buffer to decode from
         * @param [length] Message length if known beforehand
         * @returns ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decode(reader: ($protobuf.Reader|Uint8Array), length?: number): master.ServerIcons;

        /**
         * Decodes a ServerIcons message from the specified reader or buffer, length delimited.
         * @param reader Reader or buffer to decode from
         * @returns ServerIcons
         * @throws {Error} If the payload is not a reader or valid buffer
         * @throws {$protobuf.util.ProtocolError} If required fields are missing
         */
        public static decodeDelimited(reader: ($protobuf.Reader|Uint8Array)): master.ServerIcons;

        /**
         * Verifies a ServerIcons message.
         * @param message Plain object to verify
         * @returns `null` if valid, otherwise the reason why it is not
         */
        public static verify(message: { [k: string]: any }): (string|null);

        /**
         * Creates a ServerIcons message from a plain object. Also converts values to their respective internal types.
         * @param object Plain object
         * @returns ServerIcons
         */
        public static fromObject(object: { [k: string]: any }): master.ServerIcons;

        /**
         * Creates a plain object from a ServerIcons message. Also converts values to other types if specified.
         * @param message ServerIcons
         * @param [options] Conversion options
         * @returns Plain object
         */
        public static toObject(message: master.ServerIcons, options?: $protobuf.IConversionOptions): { [k: string]: any };

        /**
         * Converts this ServerIcons to JSON.
         * @returns JSON object
         */
        public toJSON(): { [k: string]: any };
    }
}
