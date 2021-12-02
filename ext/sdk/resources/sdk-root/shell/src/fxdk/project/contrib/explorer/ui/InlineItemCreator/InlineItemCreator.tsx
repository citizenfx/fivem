import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { IInlineExplorerItemCreator } from '../../explorer.itemCreate';
import { returnTrue } from 'fxdk/base/functional';
import { ShellCommands } from 'shell-api/commands';
import { InPlaceInput } from 'fxdk/ui/controls/InPlaceInput/InPlaceInput';
import sItem from '../../item.module.scss';

export interface InlineItemCreatorProps {
  creator: IInlineExplorerItemCreator,
  basePath: string,
  close(): void,
}

export const InlineItemCreator = observer(function InlineItemCreator({ creator, basePath, close }: InlineItemCreatorProps) {
  const getIcon = React.useMemo(() => {
    if (typeof creator.icon === 'function') {
      return creator.icon;
    }

    return () => creator.icon;
  }, []);

  const iconRef = React.useRef(undefined);
  if (iconRef.current === undefined) {
    iconRef.current = getIcon('');
  }

  const [icon, setIcon] = React.useState(iconRef.current);

  const validate = React.useMemo(() => {
    const validator = creator.nameValidator;

    if (!validator) {
      return returnTrue;
    }

    if (typeof validator === 'function') {
      return validator;
    }

    return (name: string) => validator.test(name);
  }, [creator]);

  const handleChange = React.useCallback((name: string) => {
    close();

    if (!name) {
      return;
    }

    ShellCommands.invoke(creator.createCommandId, basePath, name);
  }, [close, basePath]);

  const handleIntermediateChange = React.useCallback((name: string) => {
    setIcon(getIcon(name));
  }, [setIcon, getIcon]);

  return (
    <div className={sItem.children}>
      <div className={sItem.root}>
        <div className={sItem['label-container']}>
          <div className={classnames(sItem.label, sItem.renaming)}>
            <div className={sItem.icon}>
              {icon}
            </div>
            <InPlaceInput
              value=""
              onChange={handleChange}
              onIntermediateChange={handleIntermediateChange}
              className={sItem.renamer}
              validate={validate}
              placeholder={creator.placeholder}
            />
          </div>
        </div>
      </div>
    </div>
  );
});
