import debug from 'debug';

export const shellLog = debug('shell');
export const errorLog = shellLog.extend('error');

shellLog.log = console.log.bind(console);

export const logger = (moduleName: string): debug.Debugger => shellLog.extend(moduleName);
export const rootLogger = (moduleName: string): debug.Debugger => {
  const logger = debug(moduleName);
  logger.log = console.log.bind(console);

  return logger;
};

export const enableLogger = (str: string) => debug.enable(str);
