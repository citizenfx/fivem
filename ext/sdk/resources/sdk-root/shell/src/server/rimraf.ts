import * as rimrafSync from 'rimraf';
import * as util from 'util';

export const rimraf = util.promisify(rimrafSync);
