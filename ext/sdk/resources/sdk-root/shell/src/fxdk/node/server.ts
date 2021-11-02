import 'fxdk/node/bootstrap';

import './node.main';

import { getContainer } from 'backend/container-access';
import { AppService } from 'backend/app/app-service';

getContainer().get(AppService).startContributions().catch((e) => {
  console.error('Failed to start app contributions', e.message, e.stack);
});
