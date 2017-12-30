import { Observable } from 'rxjs/Observable';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/observable/fromPromise';
import 'rxjs/add/operator/bufferTime';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/mergeMap';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/finally';
import 'rxjs/add/operator/map';

import { Server, ServerIcon, PinConfig } from '../app/servers/server';

import { master } from '../app/servers/master';

// this class loosely based on https://github.com/rkusa/frame-stream
class FrameReader {
    private reader: ReadableStreamReader;

    public frame = new Subject<Uint8Array>();
    public end = new Subject<void>();

    private lastArray: Uint8Array;
    private frameLength = -1;
    private framePos = 0;

    constructor(stream: ReadableStream) {
        this.reader = stream.getReader();
    }

    public beginRead() {
        this.reader.read().then(({done, value}) => {
            if (done) {
                this.end.next();
                return;
            }

            let array: Uint8Array = value;

            while (array.length > 0) {
                const start = 4;

                if (this.lastArray) {
                    const newArray = new Uint8Array(array.length + this.lastArray.length);
                    newArray.set(this.lastArray);
                    newArray.set(array, this.lastArray.length);

                    this.lastArray = null;

                    array = newArray;
                }

                if (this.frameLength < 0) {
                    if (array.length < 4) {
                        this.lastArray = array;
                        this.beginRead();
                        return;
                    }

                    this.frameLength = array[0] | (array[1] << 8) | (array[2] << 16) | (array[3] << 24);

                    if (this.frameLength > 65535) {
                        throw new Error('A too large frame was passed.');
                    }
                }

                const end = 4 + this.frameLength - this.framePos;

                if (array.length < end) {
                    this.lastArray = array;
                    this.beginRead();
                    return;
                }

                const bit = array.slice(start, end);
                this.framePos += (end - start);

                if (this.framePos === this.frameLength) {
                    // reset
                    this.frameLength = -1;
                    this.framePos = 0;
                }

                this.frame.next(bit);

                // more in the array?
                if (array.length > end) {
                    array = array.slice(end);
                } else {
                    // continue reading
                    this.beginRead();

                    return;
                }
            }
        });
    }
}

onmessage = (e: MessageEvent) => {
    if (e.data.type === 'queryServers') {
            Observable
                .fromPromise(fetch(new Request(e.data.url)))
                .mergeMap(response => {
                    const subject = new Subject<master.IServer>();
                    const frameReader = new FrameReader(response.body, this.zone);

                    frameReader.frame
                        .subscribe(message => subject.next(master.Server.decode(message)))

                    frameReader.end.subscribe(() => subject.complete());

                    frameReader.beginRead();

                    return subject;
                })
                .bufferTime(250, null, 50)
                .finally(() => (<any>postMessage)({ type: 'serversDone' }))
                .subscribe(servers => {
                    if (servers.length) {
                        (<any>postMessage)({ type: 'addServers', servers })
                    }
                });
    }
};
