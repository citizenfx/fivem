import React from 'react';
import ReactFocusLock from 'react-focus-lock';

import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';
import { clsx } from 'cfx/utils/clsx';
import { noop } from 'cfx/utils/functional';
import { useKeyboardClose } from 'cfx/utils/hooks';
import { attachOutlet } from 'cfx/utils/outlet';

import { Button } from '../Button/Button';
import { Icons } from '../Icons';
import { Interactive } from '../Interactive/Interactive';

import s from './Flyout.module.scss';

const FlyoutOutlet = attachOutlet('overlay-outlet');

export interface FlyoutProps {
  disabled?: boolean;

  size?: 'normal' | 'small' | 'xsmall';

  onClose?(): void;
  children?: React.ReactNode;

  holderClassName?: string;

  disableSoundEffects?: boolean;
}

export function Flyout(props: FlyoutProps) {
  const {
    disabled = false,
    size = 'normal',
    onClose = noop,
    children,
    holderClassName,
    disableSoundEffects = false,
  } = props;
  const shouldPlaySounds = __CFXUI_USE_SOUNDS__ && !disabled && !disableSoundEffects;

  React.useEffect(() => {
    if (!shouldPlaySounds) {
      return;
    }

    playSfx(Sfx.Woosh1);

    return () => playSfx(Sfx.Woosh2);
  }, [shouldPlaySounds]);

  React.useEffect(() => {
    if (disabled) {
      return;
    }

    const $cfx = document.getElementById('cfxui-root');

    $cfx?.classList.add('shrink');

    return () => $cfx?.classList.remove('shrink');
  }, [disabled]);

  useKeyboardClose(onClose);

  const rootClassName = clsx(s.root, s[`size-${size}`], {
    [s.active]: !disabled,
  });

  return (
    <FlyoutOutlet>
      <div className={rootClassName}>
        <Interactive showPointer={false} className={s.backdrop} onClick={onClose} />

        <div className={s.mask}>
          <div className={clsx(s.holder, holderClassName)}>
            <ReactFocusLock disabled={!!disabled} className={s.content}>
              {children}
            </ReactFocusLock>
          </div>
        </div>
      </div>
    </FlyoutOutlet>
  );
}

export interface FlyoutHeaderProps {
  onClose?(): void;
  children: React.ReactNode;
}

Flyout.Header = React.forwardRef(function FlyoutHeader(props: FlyoutHeaderProps, ref: React.Ref<HTMLDivElement>) {
  const {
    onClose,
    children,
  } = props;

  return (
    <div ref={ref} className={s.header}>
      <div className={s.title}>{children}</div>

      {!!onClose && (
        <Button size="large" icon={Icons.exit} onClick={onClose} />
      )}
    </div>
  );
});
