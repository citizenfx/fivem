const path = require('path');
const fs = require('fs');
const child_process = require('child_process');
let buildingInProgress = false;
let currentBuildingModule = '';

const initCwd = process.cwd();
const trimOutput = (data) => {
	return `[yarn]\t` + data.toString().replace(/\s+$/, '');
}

const yarnBuildTask = {
	shouldBuild(resourceName) {
		try {
			const resourcePath = GetResourcePath(resourceName);
			
			const packageJson = path.resolve(resourcePath, 'package.json');
			const yarnLock = path.resolve(resourcePath, '.yarn.installed');
			
			const packageStat = fs.statSync(packageJson);
			
			try {
				const yarnStat = fs.statSync(yarnLock);
				
				if (packageStat.mtimeMs > yarnStat.mtimeMs) {
					return true;
				}
			} catch (e) {
				// no yarn.installed, but package.json - install time!
				return true;
			}
		} catch (e) {
			
		}
		
		return false;
	},
	
	build(resourceName, cb) {
		(async () => {
			while (buildingInProgress && currentBuildingModule !== resourceName) {
				console.log(`yarn is currently busy: we are waiting to compile ${resourceName}`);
				await sleep(3000);
			}
			buildingInProgress = true;
			currentBuildingModule = resourceName;
			const proc = child_process.fork(
				require.resolve('./yarn_cli.js'),
				['install', '--ignore-scripts', '--cache-folder', path.join(initCwd, 'cache', 'yarn-cache'), '--mutex', 'file:' + path.join(initCwd, 'cache', 'yarn-mutex')],
				{
					cwd: path.resolve(GetResourcePath(resourceName)),
					stdio: 'pipe',
				});
			proc.stdout.on('data', (data) => console.log(trimOutput(data)));
			proc.stderr.on('data', (data) => console.error(trimOutput(data)));
			proc.on('exit', (code, signal) => {
				setImmediate(() => {
					if (code != 0 || signal) {
						buildingInProgress = false;
						currentBuildingModule = '';
						cb(false, 'yarn failed!');
						return;
					}

					const resourcePath = GetResourcePath(resourceName);
					const yarnLock = path.resolve(resourcePath, '.yarn.installed');
					fs.writeFileSync(yarnLock, '');

					buildingInProgress = false;
					currentBuildingModule = '';
					cb(true);
				});
			});
		})();
	}
};

function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}
RegisterResourceBuildTaskFactory('yarn', () => yarnBuildTask);
