const path = require('path');
const { Color, getArg, exec, printError, logPrefix, pathExists, rmdir, yarn, del, print, xcopy, move, rewriteFile } = require('./utils');

const ShouldBuildFXCode = (getArg('--build-fxcode') || '').toLowerCase() === 'true';
const FXCodeCommitHash = getArg('--fxcode-commit') || 'unknown';

const SevenZip = path.join(__dirname, '../../code/tools/ci/7z');

const BuildRoot = path.join(__dirname, 'sdk-root');

const Resources = path.join(__dirname, '../sdk/resources');
const SdkGame = path.join(Resources, 'sdk-game');
const SdkRoot = path.join(Resources, 'sdk-root');

const FXCode = path.join(SdkRoot, 'fxcode');
const FXCodeFxdk = path.join(FXCode, 'fxdk');
const Shell = path.join(SdkRoot, 'shell');

async function buildGame() {
    const name = logPrefix('sdk-game', Color.FgYellow);
    const cwd = SdkGame;

    await exec({
        name,
        cmd: 'yarn',
        args: ['install', '--frozen-lockfile'],
        cwd,
    });

    await exec({
        name,
        cmd: 'yarn',
        args: ['build'],
        cwd,
    });
}

async function buildShell() {
    const name = logPrefix('shell', Color.FgGreen);
    const cwd=  Shell;

    if (await pathExists(cwd, 'build')) {
        await rmdir(name, cwd, 'build');
    }
    if (await pathExists(cwd, 'build_server')) {
        await rmdir(name, cwd, 'build_server');
    }

    await yarn(name, cwd, 'install', '--frozen-lockfile');
    await Promise.all([
        yarn(logPrefix('shell:server', Color.FgBlue), cwd, 'build:server'),
        yarn(logPrefix('shell:client', Color.FgCyan), cwd, 'build:client'),
    ]);

    try {
        await Promise.all([
            del(name, cwd, 'build\\static\\js\\*.map'),
            del(name, cwd, 'build\\static\\js\\*.txt'),
            del(name, cwd, 'build\\static\\css\\*.map'),
            del(name, cwd, 'build_server\\*.map'),
            del(name, cwd, 'build_server\\*.txt'),
        ]);
    } catch (e) {
        // doesn't matter if we can't delete these
    }
}

async function buildFXCode() {
    const name = logPrefix('fxcode', Color.FgMagenta);
    const cwd = FXCodeFxdk;

    const alreadyBuilt = await pathExists(FXCode, 'out-fxdk-pkg');

    if (!ShouldBuildFXCode && alreadyBuilt) {
        return print(name, 'Skipping FXCode build as it did not change');
    }

    await yarn(name, FXCode, 'install', '--frozen-lockfile', '--ignore-engines');
    await yarn(name, FXCode, 'download-builtin-extensions');

    await yarn(name, cwd, 'install', '--frozen-lockfile');
    await yarn(name, cwd, 'rebuild-native-modules');
    await yarn(name, cwd, 'build');

    // Writing date and commit hash
    function rewriter(content) {
        return content
            .replaceAll('@@COMMIT@@', FXCodeCommitHash)
            .replaceAll('@@DATE@@', new Date().toISOString());
    }

    print(name, 'Writing variables');
    await rewriteFile(path.join(FXCode, 'out-fxdk-pkg/out/vs/fxdk/browser/workbench/workbench.js'), rewriter);
    print(name, 'Done writing variables');
}

async function packFXCode() {
    const name = logPrefix('fxcode:pack', Color.FgMagenta);
    const cwd = FXCode;

    if (await pathExists(cwd, 'fxcode.tar')) {
        await del(name, cwd, 'fxcode.tar');
    }

    await exec({
        name,
        cwd,
        cmd: SevenZip,
        args: ['a', '-mx=0', 'fxcode.tar', 'out-fxdk-pkg\\*'],
    });
}

async function assemble() {
    const name = logPrefix('assemble', Color.BgGreen + Color.FgBlack);
    const cwd = BuildRoot;

    const resourcePath = path.join(cwd, 'resource');
    const shellPath = path.join(resourcePath, 'shell');

    if (await pathExists(cwd, 'resource')) {
        await rmdir(name, cwd, 'resource');
    }

    await xcopy(name, path.join(SdkRoot, 'fxmanifest.lua'), resourcePath + '\\');
    await xcopy(name, path.join(Shell, 'index.js'), shellPath + '\\');
    await xcopy(name, path.join(Shell, 'mpMenu.html'), shellPath + '\\');

    const fromTo = (item, from, to) => [
        path.join(from, item),
        path.join(to, item),
    ];

    await Promise.all([
        move(name, ...fromTo('build', Shell, shellPath)),
        move(name, ...fromTo('build_server', Shell, shellPath)),
        move(name, ...fromTo('fxcode.tar', FXCode, resourcePath)),
    ]);
}

Promise.resolve()
    .then(buildGame)
    .then(buildShell)
    .then(buildFXCode)
    .then(packFXCode)
    .then(assemble)
    .then(() => {
        print(`${Color.FgGreen}✓${Color.Reset}`, 'Done building!');
    })
    .catch((e) => {
        printError(`${Color.FgRed}×${Color.Reset}`, 'Build failed:', e);
        process.exit(1);
    });
