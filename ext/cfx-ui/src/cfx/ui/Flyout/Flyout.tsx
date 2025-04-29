import React from 'react';
import ReactFocusLock from 'react-focus-lock';
import { attachOutlet } from 'cfx/utils/outlet';
import { noop } from 'cfx/utils/functional';
import { clsx } from 'cfx/utils/clsx';
import { useKeyboardClose } from 'cfx/utils/hooks';
import { playSfx, Sfx } from 'cfx/apps/mpMenu/utils/sfx';
import { Flex } from '../Layout/Flex/Flex';
import { ControlBox } from '../ControlBox/ControlBox';
import { TextBlock } from '../Text/Text';
import { Button } from '../Button/Button';
import { Icons } from '../Icons';
import s from './Flyout.module.scss';

const FlyoutOutlet = attachOutlet('overlay-outlet');

export interface FlyoutProps {
  disabled?: boolean,

  size?: 'normal' | 'small' | 'xsmall',

  onClose?(): void,
  children?: React.ReactNode,

  holderClassName?: string,

  disableSoundEffects?: boolean,
}

export function Flyout(props: FlyoutProps) {
  const {
    disabled = false,
    size = 'normal',
    onClose = noop,
    children,
    holderClassName,
  } = props;

  if (__CFXUI_USE_SOUNDS__) {
    const shouldPlaySounds = !disabled;

    React.useEffect(() => {
      if (!shouldPlaySounds) {
        return;
      }

      playSfx(Sfx.Woosh1);

      return () => playSfx(Sfx.Woosh2);
    }, [shouldPlaySounds]);
  }

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
        <div className={s.backdrop} onClick={onClose} />

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
  onClose?(): void,
  children: React.ReactNode,
}

Flyout.Header = React.forwardRef((props: FlyoutHeaderProps, ref: React.Ref<HTMLDivElement>) => {
  const {
    onClose,
    children,
  } = props;

  return (
    <div ref={ref} className={s.header}>
      <div className={s.title}>
        {children}
      </div>

      {!!onClose && (
        <Button
          size="large"
          icon={Icons.exit}
          onClick={onClose}
        />
      )}
    </div>
  );
});
