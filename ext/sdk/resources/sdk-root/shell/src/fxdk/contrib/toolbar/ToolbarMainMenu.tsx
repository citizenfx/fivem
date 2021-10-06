import React from 'react';
import { ContextMenu, ContextMenuItemsCollection } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { Title } from 'fxdk/ui/controls/Title/Title';
import { devtoolsIcon } from 'constants/icons';
import { observer } from 'mobx-react-lite';
import { BsList } from 'react-icons/bs';
import { VscDebugRestart } from 'react-icons/vsc';
import { GameState } from 'store/GameState';
import { ToolbarParticipants } from './toolbarExtensions';
import s from './Toolbar.module.scss';

const handleMenuClick = (openMenu?: Function) => openMenu?.();
const handleGetMenuCoords = () => ({
  top: 0,
  left: 0,
});

export const ToolbarMainMenu = observer(function ToolbarMainMenu() {
  const menuItems: ContextMenuItemsCollection = [
    ...ToolbarParticipants.getMainMenuAdditions(),
    {
      id: 'utils',
      items: [
        {
          id: 'dev-tools',
          text: 'DevTools',
          icon: devtoolsIcon,
          onClick: () => window.openDevTools(),
        },
        {
          id: 'restart-game',
          text: 'Restart game',
          icon: <VscDebugRestart />,
          onClick: GameState.restart,
        },
      ],
    },
  ];

  return (
    <ContextMenu
      items={menuItems}
      onClick={handleMenuClick}
      getCoords={handleGetMenuCoords}
    >
      <Title animated={false} delay={0} title="Menu" fixedOn="bottom-left">
        {(ref) => (
          <button
            ref={ref}
            className={s.menu}
          >
            <BsList />
          </button>
        )}
      </Title>
    </ContextMenu>
  );
});
