const fs = require('fs');
const os = require('os');
const path = require('path');
const recursive = require('recursive-readdir');
const replaceAll = require('string.prototype.replaceall');
const td = require('typedoc');

if (!process.argv[2]) {
    console.log('usage: node index.js ../../code/');
    return;
}

(async() => {
    // STEP 1: gather source code into a TS file
    const docs = await gatherCode(process.argv[2]);

    // STEP 2: make assets
    writeDocs(docs, 'client');
    writeDocs(docs, 'server');
})();

async function gatherCode(sourceDir) {
    return new Promise((resolve, reject) => {    
        recursive(sourceDir, [ (file, stat) => !stat.isDirectory() && file.indexOf('.cpp') === -1 && file.indexOf('.h') === -1 ], (err, files) => {
            if (err) return reject(err);

            const list = [];
            const re = /^(?<spaces>\s+)(?:\/\*|--\[\[)NETEV (?<name>[^\s]*)(?: (?<type>[^\s]*))?(?<code>.*?)(?:\*\/|]])/msg;

            for (const file of files) {
                const data = fs.readFileSync(file, 'utf-8');

                const matches = [...data.matchAll(re)];

                if (matches.length > 0) {
                    list.push(...matches.map(a => a.groups));
                }
            }

            resolve(list);
        });
    });
}

async function writeDocs(docList, checkType) {
    // make docs into files
    let file = ``;
    for (const doc of docList) {
        const type = (doc.type) ? doc.type.toLowerCase() : null;

        if (type && type !== 'shared' && type !== checkType) {
            continue;
        }

        const spaces = doc.spaces.replace(/[\r\n]/g, '');
        file += replaceAll(doc.code
            .replace(/\/#/g, '/*')
            .replace(/#\//g, '*/')
            .trim(), spaces, '') + "\n\n";
    }

    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'fxed'));
    fs.writeFileSync(dir + '/main.ts', file);

    const app = new td.Application();
    app.bootstrap({
        mode: 'file',
        name: `CitizenFX events: ${checkType}`,
        disableSources: true,
        excludeTags: ['@return'],
        readme: 'none',
        experimentalDecorators: true,
    });
    app.generateDocs([ dir + '/main.ts'], `${__dirname}/out/${checkType}/`);


}