import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { FlashingMessageState } from './FlashingMessageState';
import s from './FlashingMessage.module.scss';

export const FlashingMessage = observer(function FlashingMessage() {
  const rootClassName = classnames(s.root, {
    [s.active]: FlashingMessageState.active,
  });

  return (
    <div className={rootClassName}>
      <div className={s.message}>
        {FlashingMessageState.message}
      </div>
    </div>
  );
});
