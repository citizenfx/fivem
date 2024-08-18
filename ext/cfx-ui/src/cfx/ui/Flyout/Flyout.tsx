import { Flyout as UiFlyout } from '@cfx-dev/ui-components';
import React from 'react';

import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';

export interface FlyoutProps extends React.ComponentProps<typeof UiFlyout> {
  disableSoundEffects?: boolean;
}

export function Flyout(props: FlyoutProps) {
  const {
    disableSoundEffects = false,
    ...restProps
  } = props;
  const shouldPlaySounds = __CFXUI_USE_SOUNDS__ && !restProps.disabled && !disableSoundEffects;

  React.useEffect(() => {
    if (!shouldPlaySounds) {
      return;
    }

    playSfx(Sfx.Woosh1);

    return () => playSfx(Sfx.Woosh2);
  }, [shouldPlaySounds]);

  return (
    <UiFlyout {...restProps} />
  );
}

Flyout.Header = UiFlyout.Header;
