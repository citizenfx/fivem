const http = require('http');
const mimetype = require('./mime-types');

const server = http.createServer((req, res) => {
	if (req.url === '/') {
		res.writeHead(200, { 'Content-Type': 'text/html' });
		res.end(LoadResourceFile(GetCurrentResourceName(), 'app/index.html'));
	} else {
		res.writeHead(200, {
			'Content-Type': mimetype.lookup(req.url),
		});
		res.end(LoadResourceFile(GetCurrentResourceName(), 'app' + req.url));
	}
});

server.listen(35419);

emit('sdk:openBrowser', GetConvar('sdk_url'));

setTimeout(() => {
	emit('sdk:startGame');
}, 2500);

console.log('fxDK');
console.log(process.versions);
