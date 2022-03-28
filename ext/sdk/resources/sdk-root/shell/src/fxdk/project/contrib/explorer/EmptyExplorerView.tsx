import React from 'react';
import { ImporterCommads } from 'fxdk/contrib/importer/browser/importer.commands';
import { importAssetIcon } from 'fxdk/ui/icons';
import { observer } from 'mobx-react-lite';
import { WiWindy } from 'react-icons/wi';
import { ShellCommands } from 'shell-api/commands';
import { getCreatorIcon, IExplorerItemCreator } from './explorer.itemCreate';
import { ExplorerRuntime } from './explorer.runtime';
import { ProjectExplorerParticipants } from './projectExplorerExtensions';
import s from './ProjectExplorer.module.scss';

function orderSorter(a: IExplorerItemCreator, b: IExplorerItemCreator): number {
  return (a.order || Number.MAX_SAFE_INTEGER) - (b.order || Number.MAX_SAFE_INTEGER);
}

export const EmptyExplorerView = observer(function EmptyExplorerView() {
  const itemCreatorsNodes = ProjectExplorerParticipants.getAllItemCreators().slice().sort(orderSorter).map((creator) => {
    const handleClick = () => ShellCommands.invoke(ExplorerRuntime.getOrCreateRootItemCreatorCommandId(creator));

    return (
      <button key={creator.id} onClick={handleClick}>
        {getCreatorIcon(creator)}

        New {creator.label}
      </button>
    );
  });

  return (
    <>
      <div className={s.empty}>
        <WiWindy />

        <div>
          Looks pretty empty here :(
          <br />
          <br />
          Start by creating something new
          <br />
          or import existing stuff!
        </div>
      </div>

      <div className={s['quick-access']}>
        {itemCreatorsNodes}

        <button onClick={() => ShellCommands.invoke(ImporterCommads.OPEN)}>
          {importAssetIcon}
          Import asset
        </button>
      </div>
    </>
  );
});
