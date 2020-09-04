Error.prepareStackTrace = null;

const fs = require('fs');
const path = require('path');

const http = require('http');
const mimetype = require('./mime-types');

const sdkUrl = GetConvar('sdk_url');
const selfHosted = sdkUrl === 'http://localhost:35419/';


const hostPath = path.join(process.cwd(), 'citizen/sdk/sdk-root/host');
const personalityTheiaPath = path.join(hostPath, 'personality-theia');
const shellPath = path.join(hostPath, 'shell');


if (selfHosted) {
	try {
		process.chdir(personalityTheiaPath);
		require('./host/personality-theia/server')(35420, 'localhost');
	} catch (e) {
		console.log('personality-theia has failed to start');
		console.error(e);
	}
	
	const server = http.createServer((req, res) => {
		if (req.url === '/') {
			res.writeHead(200, { 'Content-Type': 'text/html' });
			res.end(fs.readFileSync(path.join(shellPath, 'index.html')));
		} else {
			res.writeHead(200, {
				'Content-Type': mimetype.lookup(req.url),
			});
			res.end(fs.readFileSync(shellPath + req.url));
		}
	});
	
	server.listen(35419);
}

emit('sdk:openBrowser', sdkUrl);

setTimeout(() => {
	emit('sdk:startGame');
}, 2500);

console.log('fxDK');
console.log(process.versions);
