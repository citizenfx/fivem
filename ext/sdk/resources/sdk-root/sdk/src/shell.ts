import * as cors from 'cors';
import * as express from 'express';
import * as expressWs from 'express-ws';
import * as paths from './paths';
import { mountApi } from './api/api';


const shellPort = 35419;
const shellApp = express();

expressWs(shellApp);

export const startShellApp = () => new Promise((resolve) => {
  mountApi(shellApp as any as expressWs.Application);

  shellApp.use(cors({ origin: '*' }));
  shellApp.use(express.static(paths.sdkRootShell));

  shellApp.listen(shellPort, () => {
    resolve();
    console.log(`Shell server started at http://localhost:${shellPort}/`);
  });
});
