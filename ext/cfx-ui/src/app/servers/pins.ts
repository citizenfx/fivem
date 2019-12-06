export class PinConfig {
    pinIfEmpty = false;

    pinnedServers: string[] = [];
}

export class PinConfigCached {
    public data: PinConfig;
    public pinnedServers: Set<string>;

    constructor(pinConfig: PinConfig) {
        if (pinConfig) {
            this.data = pinConfig;
        } else {
            this.data = new PinConfig();
        }

        this.pinnedServers = new Set<string>(this.data.pinnedServers);
    }
}
