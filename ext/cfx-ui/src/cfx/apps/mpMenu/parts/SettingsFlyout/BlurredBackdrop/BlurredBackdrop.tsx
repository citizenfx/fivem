import { observer } from 'mobx-react-lite';
import React from 'react';

import { KnownConvars, useConvarService } from 'cfx/apps/mpMenu/services/convars/convars.service';
import { $L } from 'cfx/common/services/intl/l10n';
import { Checkbox } from 'cfx/ui/Checkbox/Checkbox';
import { Flex } from 'cfx/ui/Layout/Flex/Flex';
import { Text } from 'cfx/ui/Text/Text';

export const BlurredBackdrop = observer(function BlurredBackdrop() {
  const ConvarService = useConvarService();

  const value = ConvarService.getBoolean(KnownConvars.preferBlurredBackdrop);

  const handleChange = React.useCallback((newValue: boolean) => {
    ConvarService.setBoolean(KnownConvars.preferBlurredBackdrop, newValue);
  }, []);

  const label = (
    <Flex vertical gap="small">
      <span>{$L('#Settings_BlurredBackdropDesc')}</span>

      <Text colorToken="text-warning" size="small" weight="bold">
        {$L('#Settings_BlurredBackdropWarning')}
      </Text>
    </Flex>
  );

  return (
    <Checkbox label={label} value={value} onChange={handleChange} />
  );
});
