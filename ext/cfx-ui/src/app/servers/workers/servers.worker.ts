import { Subject, from, timer } from 'rxjs';

import { bufferTime, mergeMap, finalize, retryWhen, tap, take, delayWhen } from 'rxjs/operators';

import { master } from '../master';
import { FilterRequest } from '../filter-request';
import { PinConfigCached, PinConfig } from '../pins';
import { ServerFilterContainer } from '../components/filters/server-filter-container';
import { getCanonicalLocale } from '../components/utils';
import { filterProjectDesc, filterProjectName } from '../server-utils';

function softSlice(arr: Uint8Array, start: number, end?: number) {
    return new Uint8Array(arr.buffer, arr.byteOffset + start, end && end - start);
}

// this class loosely based on https://github.com/rkusa/frame-stream
class FrameReader {
	private reader: ReadableStreamReader<any>;

	public frame = new Subject<Uint8Array>();
	public end = new Subject<void>();

	private lastArray: Uint8Array;
	private frameLength = -1;
	private framePos = 0;

	constructor(stream: ReadableStream) {
		this.reader = stream.getReader();
	}

	public beginRead() {
		this.reader.read().then(({ done, value }) => {
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

				const bit = softSlice(array, start, end);
				this.framePos += (end - start);

				if (this.framePos === this.frameLength) {
					// reset
					this.frameLength = -1;
					this.framePos = 0;
				}

				this.frame.next(bit);

				// more in the array?
				if (array.length > end) {
					array = softSlice(array, end);
				} else {
					// continue reading
					this.beginRead();

					return;
				}
			}
		});
	}
}

function isPinned(pinConfig: PinConfigCached, server: master.IServer) {
	if (!pinConfig || !pinConfig.pinnedServers) {
		return false;
	}

	return pinConfig.pinnedServers.has(server.EndPoint);
}

function quoteRe(text: string) {
	return text.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

const sortNames: { [key: string]: string } = {};
const stripNames: { [key: string]: string } = {};

function stripName(server: master.IServerData) {
	let name = server.hostname || '';

	if (server.vars?.sv_projectName) {
		name = filterProjectName(server.vars?.sv_projectName) + ' ' + filterProjectDesc(server.vars?.sv_projectDesc ?? '');
	} else {
		if (name.length >= 100) {
			name = name.substring(0, 100);
		}
	}

	return (name).replace(/\^[0-9]/g, '').normalize('NFD').replace(/[\u0300-\u036f]/g, '')
}

function sortName(server: master.IServerData) {
	return stripName(server).replace(/[^a-zA-Z0-9]/g, '').replace(/^[0-9]+/g, '').toLowerCase();
}

function buildSearchMatch(filterList: ServerFilterContainer) {
	const filters = filterList.filters;
	const searchText = filters.searchText;
	const filterFns: ((server: master.IServer) => boolean)[] = [];

	const searchRe = /((?:~?\/.*?\/)|(?:[^\s]+))\s?/g;
	const categoryRe = /^([^:]*?):(.*)$/

	const isFalse = a => !a || a === 'false' || a === '0';

	let match: RegExpExecArray;

	while (match = searchRe.exec(searchText)) {
		let searchGroup = match[1];
		let invertSearch = false;

		if (searchGroup.startsWith('~')) {
			searchGroup = searchGroup.substring(1);
			invertSearch = true;
		}

		if (searchGroup.length < 2) {
			continue;
		}

		const categoryMatch = searchGroup.match(categoryRe);

		if (!categoryMatch) {
			const reString =
				(searchGroup.match(/^\/(.+)\/$/)) ?
					searchGroup.replace(/^\/(.+)\/$/, '$1')
					:
					quoteRe(searchGroup);

			try {
				const re = new RegExp(reString, 'i');
				filterFns.push(a => (invertSearch) ? !re.test(stripNames[a.EndPoint]) : re.test(stripNames[a.EndPoint]));
			} catch (e) { }
		} else {
			const category = categoryMatch[1];
			const match = new RegExp(quoteRe(categoryMatch[2]), 'i');

			filterFns.push(server => {
				let result = false;

				if (server.Data[category]) {
					result = match.test(String(server.Data[category]));
				} else if (server.Data[category + 's']) {
					const fields = server.Data[category + 's'];

					if (Array.isArray(fields)) {
						result = fields.filter(a => match.test(String(a))).length > 0;
					} else {
						const values = Object.keys(fields).filter(a => match.test(String(a)));

						if (values.length > 0) {
							if (!isFalse(fields[values[0]])) {
								result = true;
							}
						}
					}
				} else if (server.Data.vars[category + 's']) {
					const fields = (<string>server.Data.vars[category + 's']).split(',');

					result = fields.filter(a => match.test(String(a))).length > 0;
				} else if (server.Data.vars[category]) {
					result = match.test(String(server.Data.vars[category]));
				}

				if (invertSearch) {
					result = !result;
				}

				return result;
			});
		}
	}

	return (server: master.IServer) => {
		for (const fn of filterFns) {
			if (!fn(server)) {
				return false;
			}
		}

		return true;
	};
}

function getFilter(pinConfig: PinConfigCached, filterList: ServerFilterContainer, listType: string): (server: master.IServer) => boolean {
	const nameMatchCallback = buildSearchMatch(filterList);
	const filters = filterList.filters;

	const hiddenByTags = (server: master.IServer) => {
		const tagListEntries = (filterList.tags) ? Object.entries(filterList.tags.tagList) : [];

		if (tagListEntries.length > 0) {
			const tags =
				(server && server.Data && server.Data.vars && server.Data.vars.tags) ?
					(<string>server.Data.vars.tags)
						.split(',')
						.map(a => a.trim().toLowerCase())
						.filter(a => a)
					:
					[];

			const tagSet = new Set<string>(tags);

			for (const [tag, active] of tagListEntries) {
				if (active) {
					if (!tagSet.has(tag)) {
						return true;
					}
				} else {
					if (tagSet.has(tag)) {
						return true;
					}
				}
			}
		}

		return false;
	};

	const hiddenByLocales = (server: master.IServer) => {
		const localeListEntries = (filterList.tags) ? Object.entries(filterList.tags.localeList) : [];

		let matchesLocales = true;

		if (localeListEntries.length > 0) {
			const sl = server?.Data?.vars?.locale && getCanonicalLocale(server.Data.vars.locale);

			matchesLocales = false;

			for (const [locale, active] of localeListEntries) {
				if (active) {
					if (sl === locale) {
						matchesLocales = true;
					}
				} else {
					if (sl === locale) {
						return true;
					}

					matchesLocales = true;
				}
			}
		}

		return !matchesLocales;
	};

	const shownAsPin = (server: master.IServer) => (isPinned(pinConfig, server) && listType === 'browse');

	return (server) => {
		if (!nameMatchCallback(server)) {
			return false;
		}

		if (server.Data.clients === 0 && filters.hideEmpty) {
			if (!isPinned(pinConfig, server) || !pinConfig.data.pinIfEmpty) {
				return false;
			}
		}

		if (server.Data.clients >= server.Data.svMaxclients && filters.hideFull) {
			return false;
		}

		if (filterList.tags.tagList) {
			if (hiddenByTags(server) && !shownAsPin(server)) {
				return false;
			}
		}

		if (filterList.tags.localeList) {
			if (hiddenByLocales(server) && !shownAsPin(server)) {
				return false;
			}
		}

		return true;
	}
}

function getSortable(server: master.IServer, name: string): any {
	switch (name) {
		case 'name':
			return sortNames[server.EndPoint];
		case 'ping':
			return 0;
		case 'players':
			return server.Data.clients || 0;
		case 'upvotePower':
			return server.Data.upvotePower || 0;
		default:
			throw new Error('Unknown sortable');
	}
}

class ServerTagsWorker {
	private serverTags: { [addr: string]: true } = {};
	private serverLocale: { [addr: string]: true } = {};

	private tagsIndex: { [key: string]: number } = {};
	private localesIndex: { [key: string]: number } = {};

	send() {
		return [this.tagsIndex, this.localesIndex];
	}

	addServerTags(server: master.IServer) {
		if (this.serverTags[server.EndPoint]) {
			return;
		}

		if (server.Data?.vars?.tags) {
			(<string>server.Data.vars.tags)
				.split(',')
				.forEach((rawTag) => {
					const tag = rawTag.trim().toLowerCase();

					if (!tag) {
						return;
					}

					this.tagsIndex[tag] = (this.tagsIndex[tag] || 0) + 1;
				});

			this.serverTags[server.EndPoint] = true;
		}
	}

	addLocaleIndex(server: master.IServer) {
		if (this.serverLocale[server.EndPoint]) {
			return;
		}

		if (server?.Data?.vars?.locale) {
			this.serverLocale[server.EndPoint] = true;

			const locale = getCanonicalLocale(server.Data.vars.locale);

			// We don't care about root, right
			if (locale === 'root-AQ') {
				return;
			}

			this.localesIndex[locale] = (this.localesIndex[locale] || 0) + 1;
		}
	}
}

interface SearchAutocompleteIndex {
	[type: string]: {
		[entry: string]: number,
	},
};

class ServerAutoCompleteWorker {
	autocompleteIndex: SearchAutocompleteIndex = {};
	autocompleteIndexBacking: SearchAutocompleteIndex = {};

	addAutocompleteIndex(server: master.IServer) {
		for (const list of [server.Data, server.Data?.vars]) {
			for (const entryName in list) {
				if (list.hasOwnProperty(entryName)) {
					const key = entryName;
					const fields = list[entryName];

					if (key.endsWith('s')) {
						const bits: string[] = [];

						const subKey = key.substr(0, key.length - 1);

						if (Array.isArray(fields)) {
							for (const item of fields) {
								bits.push(item);
							}
						} else if (typeof fields === 'object') {
							const values = Object.keys(fields);

							for (const item of values) {
								bits.push(item);
							}
						} else if (typeof fields === 'string') {
							for (const item of fields.split(',')) {
								bits.push(item.trim());
							}
						}

						const uniqueBits = bits.filter((v, i, a) => a.indexOf(v) === i && typeof (v) === 'string');
						this.addAutoCompleteEntries(subKey, uniqueBits);
					}

					if (typeof fields === 'string') {
						this.addAutoCompleteEntries(key, [fields]);
					}
				}
			}
		}
	}

	addAutoCompleteEntries(key: string, list: string[]) {
		if (!this.autocompleteIndexBacking[key]) {
			this.autocompleteIndexBacking[key] = {};
		}

		for (const entry of list) {
			const lowerEntry = entry.toLowerCase();

			if (!this.autocompleteIndexBacking[key][lowerEntry]) {
				this.autocompleteIndexBacking[key][lowerEntry] = 1;
			} else {
				if (!this.autocompleteIndex[key]) {
					this.autocompleteIndex[key] = {};
				}

				this.autocompleteIndex[key][lowerEntry] = ++this.autocompleteIndexBacking[key][lowerEntry];
			}
		}
	}
}

const serverTagsWorker = new ServerTagsWorker();
const serverAutoCompleteWorker = new ServerAutoCompleteWorker();

let cachedServers = {};

function queryServers(e: MessageEvent) {
	cachedServers = {};

	from(fetch(new Request(e.data.url)))
		.pipe(
			mergeMap(response => {
				const subject = new Subject<master.IServer>();
				const frameReader = new FrameReader(response.body);

				frameReader.frame
					.subscribe(message => subject.next(master.Server.decode(message)))

				frameReader.end.subscribe(() => subject.complete());

				frameReader.beginRead();

				return subject;
			}),
			bufferTime(250, null, 250),
			retryWhen(errors =>
				errors.pipe(
					tap(err => console.log(`Fetching server list failed: ${err}`)),
					delayWhen(() => timer(2000)),
					take(10)
				)),
			finalize(() => (<any>postMessage)({ type: 'serversDone' }))
		)
		.subscribe((servers: master.IServer[]) => {
			if (servers.length) {
                for (const server of servers) {
					if (server.Data?.vars?.sv_projectName) {
						server.Data.vars.sv_projectName = filterProjectName(server.Data.vars.sv_projectName);
					}

					if (server.Data?.vars?.sv_projectDesc) {
						server.Data.vars.sv_projectDesc = filterProjectDesc(server.Data.vars.sv_projectDesc);
					}

					serverTagsWorker.addServerTags(server);
					serverTagsWorker.addLocaleIndex(server);
					serverAutoCompleteWorker.addAutocompleteIndex(server);

					cachedServers[server.EndPoint] = server.Data;
				}

				postMessage({ type: 'addServers', servers });
				postMessage({ type: 'updateTags', data: serverTagsWorker.send() });
				postMessage({ type: 'updateAutoComplete', data: serverAutoCompleteWorker.autocompleteIndex });
			}
		});
}

function sortServers(e: MessageEvent) {
	const filterRequest: FilterRequest = e.data.data;
	const pinConfig = new PinConfigCached(filterRequest.pinConfig as unknown as PinConfig);

	for (const [addr, data] of Object.entries(cachedServers)) {
		sortNames[addr] = sortName(data);
		stripNames[addr] = stripName(data);
	}

	const servers = (Object.entries(cachedServers)
		.map(([EndPoint, Data]) => ({
			EndPoint,
			Data,
		})) || [])
		.filter(getFilter(pinConfig, filterRequest.filters, filterRequest.listType));

	const sortChain = (a: master.IServer, b: master.IServer, ...stack: ((a: master.IServer, b: master.IServer) => number)[]) => {
		for (const entry of stack) {
			const retval = entry(a, b);

			if (retval !== 0) {
				return retval;
			}
		}

		return 0;
	};

	const sortSortable = (sortable: string[]) => {
		const name = sortable[0];
		const invert = (sortable[1] === '-');

		const sort = (a: master.IServer, b: master.IServer) => {
			const val1 = getSortable(a, name);
			const val2 = getSortable(b, name);

			if (val1 > val2) {
				return 1;
			}

			if (val1 < val2) {
				return -1;
			}

			return 0;
		};

		if (invert) {
			return (a: master.IServer, b: master.IServer) => -(sort(a, b));
		} else {
			return sort;
		}
	};

	const sortList = [
		(a: master.IServer, b: master.IServer) => {
			if (filterRequest.listType === 'browse' &&
				(filterRequest.filters.filters.searchText ?? '') === '') {
				return 0;
			}

			const aPinned = isPinned(pinConfig, a);
			const bPinned = isPinned(pinConfig, b);

			if (aPinned === bPinned) {
				return 0;
			} else if (aPinned && !bPinned) {
				return -1;
			} else if (!aPinned && bPinned) {
				return 1;
			}
		},
		sortSortable(filterRequest.sortOrder),
		sortSortable(['upvotePower', '-']),
		sortSortable(['players', '-']),
		sortSortable(['name', '+'])
	];

	servers.sort((a, b) => {
		return sortChain(
			a,
			b,
			...sortList
		);
	});

	(<any>postMessage)({
		type: 'sortedServers',
		servers: servers.map((a: master.IServer) => a.EndPoint)
	});
}

onmessage = (e: MessageEvent) => {
	switch (e.data.type) {
		case 'queryServers':
			queryServers(e);
			break;
		case 'sort':
			sortServers(e);
			break;

		default:
			console.error('Unknown server worker message', e);
	}
};
