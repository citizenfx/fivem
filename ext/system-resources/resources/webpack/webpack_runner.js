const webpack = require('webpack');
const path = require('path');
const fs = require('fs');

function getStat(path) {
	try {
		const stat = fs.statSync(path);

		return stat ? {
			mtime: stat.mtimeMs,
			size: stat.size,
			inode: stat.ino,
		} : null;
	} catch {
		return null;
	}
}

class SaveStatePlugin {
	constructor(inp) {
		this.cache = [];
		this.cachePath = inp.cachePath;
	}

	apply(compiler) {
		compiler.hooks.afterCompile.tap('SaveStatePlugin', (compilation) => {
			for (const file of compilation.fileDependencies) {
				this.cache.push({
					name: file,
					stats: getStat(file)
				});
			}
		});

		compiler.hooks.done.tap('SaveStatePlugin', (stats) => {
			if (stats.hasErrors()) {
				return;
			}

			fs.writeFile(this.cachePath, JSON.stringify(this.cache), () => {

			});
		});
	}
}

module.exports = (inp, callback) => {
	const config = require(inp.configPath);
	
	config.context = inp.resourcePath;
	
	if (config.output && config.output.path) {
		config.output.path = path.resolve(inp.resourcePath, config.output.path);
	}

	if (!config.plugins) {
		config.plugins = [];
	}

	config.plugins.push(new SaveStatePlugin(inp));
	
	webpack(config, (err, stats) => {
		if (err) {
			callback(err);
			return;
		}
		
		if (stats.hasErrors()) {
			callback(null, stats.toJson());
			return;
		}
		
		callback(null, {});
	});
};