import React from 'react';
import { ShellState } from 'store/ShellState';
import { ToolbarState } from 'store/ToolbarState';
import { observer } from 'mobx-react-lite';
import { FXCodeState } from './FXCodeState';
import { FXCodeImporter } from './FXCodeImporter/FXCodeImporter';
import s from './FXCodePersonality.module.scss';

export const FXCodePersonality = observer(function FXCodePersonality() {
  return (
    <>
      <iframe
        ref={FXCodeState.ref}
        style={ToolbarState.cssVariables}
        className={s.root}
        title="FXCode personality"
        src={FXCodeState.iframeSrc}
        frameBorder="0"
        allowFullScreen={true}
        data-intro-id="fxcode"
      ></iframe>

      {ShellState.isIframeCovered && (
        <div className={s.cover} style={ToolbarState.cssVariables} />
      )}

      {FXCodeState.importerDialogState.isOpen && FXCodeState.importConfigDraft && (
        <FXCodeImporter draft={FXCodeState.importConfigDraft} />
      )}
    </>
  );
});
