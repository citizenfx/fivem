export class Server {
    readonly address: string;
    readonly hostname: string;
    currentPlayers: number;

    get maxPlayers(): number {
        return 24;
    }

    get ping(): number {
        return 42;
    }

    get iconUri(): string {
        return 'http://5r.kngrektor.com/servericon/' + this.address;
    }

    public static fromObject(address: string, object: any): Server {
        return new Server(address, object);
    }

    private constructor(address: string, object: any) {
        this.address = address;
        this.hostname = object.hostname;
        this.currentPlayers = object.clients;
    }
}
