import { observer } from 'mobx-react-lite';

import { usePlatformStatusService } from 'cfx/apps/mpMenu/services/platformStatus/platformStatus.service';

import s from './PlatformStatus.module.scss';

export const PlatformStatus = observer(function PlatformStatus() {
  const PlatformStatusService = usePlatformStatusService();

  return (
    <div className={s['service-notice']}>{PlatformStatusService.serviceNotice}</div>
  );
});
