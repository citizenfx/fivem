/**
 * System Resources artifacts and manifest manager
 */
const fs = require('fs');
const http = require('http');
const https = require('https');
const crypto = require('crypto');

const MANIFEST_FILE = './manifest.json';

main(parseArgs(process.argv.slice(2))).catch((error) => {
    console.error(error);
    process.exit(1);
});

/**
 * @param {{ args: string[], options: Record<string, string | boolean> }} param0 
 * @returns 
 */
async function main({ args, options }) {
    const [cmd] = args;

    const resourceName = check(options.name, 'No --name provided');

    switch (cmd) {
        case 'download': {
            const skipHashVerification = Boolean(options.noverify);

            const resourceFile = check(options.file, 'No --file provided');

            // Get manifest and check that we have passed resource name in it
            const manifest = require(MANIFEST_FILE);
            if (!manifest[resourceName]) {
                return exitError('Resource name unknown');
            }

            // Get resource manifest
            const resourceManifest = manifest[resourceName];

            // Download resource file
            await download(resourceManifest.url, resourceFile);

            // Check if resource file at least isn't empty
            const resourceFileStat = await getFileStatSafe(resourceFile);
            if (resourceFileStat && resourceFileStat.size === 0) {
                return exitError(`Downloaded resource ${resourceName} file is empty`);
            }

            if (skipHashVerification) {
                console.log(`Resource file ${resourceFile} for ${resourceName} has been downloaded`);
            } else {
                // Verify resource file SHA256 hashsum against the one in resource' manifest
                const actualSHA256 = await getSHA256(resourceFile);

                if (resourceManifest.sha256 !== actualSHA256) {
                    console.log(`Invalid SHA256 of resource ${resourceName} file ${resourceFile}`);
                    console.log('Expected:', resourceManifest.sha256);
                    console.log('Actual:', actualSHA256);

                    return exitError('Resource file integrity check failed');
                }
    
                console.log(`Resource file ${resourceFile} for ${resourceName} has been downloaded and verified`);
            }

            break;
        }

        case 'add':
        case 'update': {
            const resourceUrl = check(options.url, 'No --url provided');
            const resourceFile = check(options.file || `${resourceName}.zip`);
            const resourceVersion = check(options.version, 'No --version provided');

            const manifest = require(MANIFEST_FILE);
            const resourceManifest = manifest[resourceName] || {};
            const isResourceManifestNew = Object.keys(resourceManifest).length === 0;

            if (cmd === 'add') {
                if (!isResourceManifestNew) {
                    return exitError('Cannot add resource as it already exists, try updating instead');
                }
            } else {
                if (isResourceManifestNew) {
                    return exitError('Resource name unknown, see manifest.json for existing resources');
                }
            }

            // Add resourceManifest to manifest in case it's new
            manifest[resourceName] = resourceManifest;

            // Download temporary resource' artifact
            await download(resourceUrl, resourceFile);

            // Compute artifact hash
            const sha256 = await getSHA256(resourceFile);

            // Delete temporary resource' artifact
            await fs.promises.unlink(resourceFile);

            // Update resource manifest
            resourceManifest.url = resourceUrl;
            resourceManifest.version = resourceVersion;
            resourceManifest.sha256 = sha256;

            // Write updated manifest
            await fs.promises.writeFile(MANIFEST_FILE, JSON.stringify(manifest, null, 2));

            console.log(`Updated resource ${resourceName} manifest:`, resourceManifest);

            break;
        }
    }
}

/**
 * Checks if input is a string and it is not empty
 * 
 * @param {string | boolean} inp 
 * @param {string} error 
 * @returns {string}
 */
function check(inp, error) {
    if (typeof inp !== 'string') {
        return exitError(`Expected string value, got ${typeof inp}`);
    }

    if (!inp) {
        return exitError(error);
    }

    return inp;
}

function exitError(message, code = 1) {
    console.error(message);
    process.exit(code);
}

/**
 * Returns hex-encoded SHA256 hashsum of the file content
 * 
 * @param {string} filename 
 * @returns {Promise<string>}
 */
async function getSHA256(filename) {
    const fileStat = await getFileStatSafe(filename);
    if (!fileStat || fileStat.isDirectory()) {
        return exitError(`${filename} does not exists or is a directory`);
    }

    return new Promise((resolve, reject) => {
        const hash = crypto.createHash('sha256');
        const fileStream = fs.createReadStream(filename);
    
        fileStream.on('data', (data) => {
            hash.update(data);
        })
        fileStream.on('end', () => {
            resolve(hash.digest('hex'));
        });
        fileStream.on('error', (err) => {
            reject(err);
        });
    });
}

/**
 * Returns fs.Stats object if file/directory exists, null otherwise
 * 
 * Does the same thing as fs.promises.stat, but doesn't throw if stat operation was not successfull
 * 
 * @param {string} filename 
 * @returns {Promise<fs.Stats | null>}
 */
async function getFileStatSafe(filename) {
    return fs.promises.stat(filename).catch(() => null);
}

/**
 * Checks if file/directory exists
 * 
 * @param {string} filename 
 * @returns {Promise<fs.Stats>}
 */
async function fileExists(filename) {
    const fileStat = await getFileStatSafe(filename);

    return fileStat !== null;
}

/**
 * Simple downloader, will follow redirects
 * 
 * @param {string} url
 * @param {string} filename 
 */
async function download(url, filename, redirectsCount = 0) {
    const DOWNLOAD_MAX_REDIRECTS = 50;

    // If not redirect - make sure file doesn't exist
    if (redirectsCount === 0 && await fileExists(filename)) {
        await fs.promises.unlink(filename);
    }

    if (redirectsCount > DOWNLOAD_MAX_REDIRECTS) {
        throw new Error(`Failed to download, maximum amount of redirects (${DOWNLOAD_MAX_REDIRECTS}) exceeded`);
    }

    const isHttp = url.startsWith('http://');
    const isHttps = url.startsWith('https://');

    if (!isHttp && !isHttps) {
        throw new Error('Unknown download url protocol, only http and https is supported');
    }

    const protocol = isHttp ? http : https;

    await new Promise((resolve, reject) => {
        protocol.get(url, (res) => {
            if (res.statusCode >= 200 && res.statusCode < 300) {
                const fileStream = fs.createWriteStream(filename);

                fileStream.on('error', reject);
                fileStream.on('finish', () => {
                    fileStream.close();
                    resolve();
                });

                res.pipe(fileStream);
            } else if (res.headers.location) {
                // If redirect - recurse with a given location
                download(res.headers.location, filename, redirectsCount + 1).then(resolve, reject);
            } else {
                reject(new Error(res.statusCode + ' ' + res.statusMessage));
            }
        });
    });
}

/**
 * Parse args array
 * 
 * @param {string[]} args 
 * @returns {{ args: string[], options: Record<string, string | boolean> }}
 */
function parseArgs(args) {
    const map = {
        args: [],
        options: {},
    };

    for (const arg of args) {
        // Doesn't start with `--` - arbitrary arg
        if (!arg.startsWith('--')) {
            map.args.push(arg);
            continue;
        }

        const argNormalized = arg.substring(2);

        // If arg has no value specified - treat it like a boolean flag
        if (!argNormalized.includes('=')) {
            map.options[argNormalized] = true;
            continue;
        }

        const [argName, argValue] = argNormalized.split('=');
        map.options[argName] = argValue;
    }

    return map;
}
