import { observer } from 'mobx-react-lite';

import { AuxiliaryGameModes } from './AuxiliaryGameModes/AuxiliaryGameModes';
import { ExtraLinkyTiles } from './ExtraLinkyTiles/ExtraLinkyTiles';
import { FeaturedServerTile, useFeaturedServer } from './FeaturedServerTile/FeaturedServerTile';

import s from './Footer.module.scss';

export const Footer = observer(function Footer() {
  const hasFeaturedServer = Boolean(useFeaturedServer());

  return (
    <div className={s.root}>
      {!!hasFeaturedServer && (
        <div className={s.featured}>
          <FeaturedServerTile />
        </div>
      )}

      <div className={s.storyreplay}>
        <AuxiliaryGameModes />
      </div>

      <div className={s.host}>
        <ExtraLinkyTiles />
      </div>
    </div>
  );
});
