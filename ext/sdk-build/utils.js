const fs = require('fs');
const cp = require('child_process');
const path = require('path');

const Color = {
    Reset: "\x1b[0m",
    Bright: "\x1b[1m",
    Dim: "\x1b[2m",
    Underscore: "\x1b[4m",
    Blink: "\x1b[5m",
    Reverse: "\x1b[7m",
    Hidden: "\x1b[8m",

    FgBlack: "\x1b[30m",
    FgRed: "\x1b[31m",
    FgGreen: "\x1b[32m",
    FgYellow: "\x1b[33m",
    FgBlue: "\x1b[34m",
    FgMagenta: "\x1b[35m",
    FgCyan: "\x1b[36m",
    FgWhite: "\x1b[37m",

    BgBlack: "\x1b[40m",
    BgRed: "\x1b[41m",
    BgGreen: "\x1b[42m",
    BgYellow: "\x1b[43m",
    BgBlue: "\x1b[44m",
    BgMagenta: "\x1b[45m",
    BgCyan: "\x1b[46m",
    BgWhite: "\x1b[47m",
};

const ErrorString = `${Color.FgRed}ERR${Color.Reset}`;

function print(...args) {
    console.log.apply(console, [
        `${Color.Dim}[${new Date().toISOString()}]${Color.Reset}`,
        ...args,
    ]);
}
function printError(...args) {
    console.error.apply(console, [
        `${Color.FgRed}[${new Date().toISOString()}]${Color.Reset}`,
        ...args,
    ]);
}
function logPrefix(name, color) {
    return `${color}[${name}]${Color.Reset}`;
}
function modulePrint(name, color) {
    return print.bind(null, logPrefix(name, color));
}

const argv = [...process.argv].reverse();
function getArg(name) {
    const arg = argv.find((argItem) => argItem.startsWith(name));

    if (!arg) {
        return;
    }

    return arg.substr(name.length + 1);
}

function exec({ cmd, args, name, cwd }) {
    const format = (text) => text.replace(/\u2026/g, '...').split('\n').filter(x => x.trim());//.filter(x => x !== '\r' || x);
    
    return new Promise((resolve, reject) => {
        print(name, path.resolve(cwd) + '$', cmd, ...args);

        const proc = cp.spawn(cmd, args, {
            cwd,
            env: process.env,
            shell: true,
            windowsVerbatimArguments: true,
        });

        proc.stdout.on('data', (data) => format(data.toString()).forEach((line) => {
            print(name, `[${cmd}]`, line);
        }));
        proc.stderr.on('data', (data) => format(data.toString()).forEach((line) => {
            printError(name, `[${cmd}]`, ErrorString, line);
        }));

        proc.on('close', (code, signal) => {
            if (code !== 0) {
                reject(`"${cmd} ${args.join(' ')}" exit code: ${code}, signal: ${signal}`);
            } else {
                resolve();
            }
        });
    });
}

async function pathExists(cwd, ...args) {
    try {
        await fs.promises.stat(path.join(cwd, ...args));

        return true;
    } catch (e) {
        return false;
    }
}

async function statSafe(cwd, ...args) {
    try {
        return await fs.promises.stat(path.join(cwd, ...args));
    } catch (e) {
        return null;
    }
}

function rmdir(name, cwd, dirPath) {
    return exec({
        name,
        cwd,
        cmd: 'rmdir',
        args: ['/s', '/q', dirPath],
    });
}

function del(name, cwd, pattern) {
    return exec({
        name,
        cwd,
        cmd: 'del',
        args: ['/q', '/f', '/s', pattern],
    });
}

function yarn(name, cwd, command, ...args) {
    return exec({
        name,
        cwd,
        cmd: 'yarn',
        args: [command, ...args],
    });
}

function xcopy(name, from, to) {
    return exec({
        name,
        cwd: '.',
        cmd: 'xcopy',
        args: ['/y', from, to],
    });
}

function move(name, from, to) {
    return exec({
        name,
        cwd: '.',
        cmd: 'move',
        args: [from, to],
    });
}

async function rewriteFile(filePath, rewriter) {
    const fileStat = await statSafe(filePath);

    if (!fileStat) {
        throw new Error(`Failed to rewrite ${filePath}, file does not exist`);
    }
    if (fileStat.isDirectory()) {
        throw new Error(`Failed to rewrite ${filePath}, not a file`);
    }

    const content = (await fs.promises.readFile(filePath)).toString();

    await fs.promises.writeFile(filePath, rewriter(content));
}

module.exports = {
    Color,
    print,
    printError,
    modulePrint,
    logPrefix,
    getArg,
    exec,
    pathExists,
    rmdir,
    del,
    yarn,
    xcopy,
    move,
    rewriteFile,
};
