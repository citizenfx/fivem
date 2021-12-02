import React from 'react';
import s from './FXCodeImporter.module.scss';
import { observer } from 'mobx-react-lite';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { ImportConfigDraft } from './ImportConfigDraft';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import { BsExclamationCircle } from 'react-icons/bs';
import { Indicator } from 'fxdk/ui/Indicator/Indicator';
import { infoIcon } from 'fxdk/ui/icons';
import { FXCodeState } from '../FXCodeState';

export interface FXCodeImportProps {
  draft: ImportConfigDraft,
}

export const FXCodeImporter = observer(function FXCodeImporter(props: FXCodeImportProps) {
  const { draft } = props;

  const settingNodes = Object.entries(draft.config.settings).map(([key, value]) => {
    const onChange = (included: boolean) => draft.setSettingIncluded(key, included);
    const included = draft.isSettingIncluded(key);

    return (
      <Checkbox
        key={key}
        className={s.toggle}
        value={included}
        onChange={onChange}
        label={`${key} = ${value}`}
      />
    );
  });

  const keybindingNodes = draft.config.keybindings.map((keybinding, index) => {
    const onChange = (included: boolean) => draft.setKeybindingIncluded(index, included);
    const included = draft.isKeybindIncluded(index);

    return (
      <Checkbox
        key={index}
        className={s.toggle}
        value={included}
        onChange={onChange}
        label={`${keybinding.key} => ${keybinding.command}`}
      />
    );
  });

  const extensionNodes = Object.keys(draft.config.extensions).map((id) => {
    const onChange = (included: boolean) => draft.setExtensionIncluded(id, included);
    const included = draft.isExtensionIncluded(id);

    return (
      <Checkbox
        key={id}
        className={s.toggle}
        value={included}
        onChange={onChange}
        label={(
          <>
            <span>{id}</span>
            &nbsp;
            <a href={`https://marketplace.visualstudio.com/items?itemName=${id}`}>
              {infoIcon}
            </a>
          </>
        )}
      />
    );
  });

  return (
    <Modal fullWidth>
      <div className={s.root}>
        <div className="modal-header">VSCode import</div>

        <div className={s.content}>
          <ScrollContainer className={s.content}>
            {!!settingNodes.length && (
              <div className="modal-block">
                <div className={s.toggles}>
                  <details>
                    <summary>
                      Import settings ({settingNodes.length} items)
                    </summary>

                    <Checkbox
                      className={s.toggleAll}
                      value={draft.allSettingsIncluded}
                      onChange={() => draft.toggleAllSettingsIncluded()}
                      label="Toggle All"
                    />

                    {settingNodes}
                  </details>
                </div>
              </div>
            )}

            {!!keybindingNodes.length && (
              <div className="modal-block">
                <div className={s.toggles}>
                  <details>
                    <summary>
                      Import key bindings ({keybindingNodes.length} items)
                    </summary>

                    <Checkbox
                      className={s.toggleAll}
                      value={draft.allKeybindingsIncluded}
                      onChange={() => draft.toggleAllKeybindingsIncluded()}
                      label="Toggle All"
                    />
                    {keybindingNodes}
                  </details>
                </div>
              </div>
            )}

            {!!extensionNodes.length && (
              <div className="modal-block">
                <div className={s.toggles}>
                  <details open>
                    <summary>
                      Import extensions ({extensionNodes.length} items)
                    </summary>

                    <Checkbox
                      className={s.toggleAll}
                      value={draft.allExtensionsIncluded}
                      onChange={() => draft.toggleAllExtensionsIncluded()}
                      label="Toggle All"
                    />

                    {extensionNodes}
                  </details>
                </div>
              </div>
            )}
          </ScrollContainer>
        </div>

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Import"
            onClick={FXCodeState.processImportConfig}
          />

          <Button
            text="Cancel and don't ask again"
            onClick={FXCodeState.closeImporter}
          />
        </div>
      </div>
    </Modal>
  );
});
