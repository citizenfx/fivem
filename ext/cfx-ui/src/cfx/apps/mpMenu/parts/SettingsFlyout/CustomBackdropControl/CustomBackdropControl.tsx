import {
  Button,
  Icons,
  Flex,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { getCustomInGameBackdropPath } from 'cfx/apps/mpMenu/parts/ThemeManager/backdrop';
import { $L } from 'cfx/common/services/intl/l10n';
import { useUiService } from 'cfx/common/services/ui/ui.service';

import { mpMenu } from '../../../mpMenu';
import { KnownConvars, useConvarService } from '../../../services/convars/convars.service';

import s from './CustomBackdropControl.module.scss';

export const CustomBackdropControl = observer(() => {
  const UiService = useUiService();
  const ConvarService = useConvarService();

  const aspectRatio = (UiService.viewportWidth / UiService.viewportHeight).toString();

  const backdropPath = ConvarService.get(KnownConvars.customBackdrop);
  const hasCustomBackdropSet = !!backdropPath;

  const onClick = async () => {
    const filePath = await mpMenu.selectFile('backdrop');

    ConvarService.set(KnownConvars.customBackdrop, filePath);
  };

  return (
    <Flex vertical>
      <Flex>
        <Button text="Select" onClick={onClick} />

        {hasCustomBackdropSet && (
          <Title fixedOn="right" title={$L('#Settings_CustomBackdropReset')}>
            <Button
              theme="transparent"
              icon={Icons.remove}
              onClick={() => ConvarService.set(KnownConvars.customBackdrop, '')}
            />
          </Title>
        )}
      </Flex>

      <div
        className={s.preview}
        style={{
          aspectRatio,
          backgroundImage: `url(${getCustomInGameBackdropPath(backdropPath)})`,
        }}
      />
    </Flex>
  );
});
