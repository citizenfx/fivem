import React from 'react';
import { WEEntityMatrixIndex } from 'backend/world-editor/world-editor-types';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { applyScale, eulerFromMatrix, getScale } from 'shared/math';
import { BsCameraVideo } from 'react-icons/bs';
import { patchesToolIcon, rotateIcon, scaleIcon, translateIcon } from 'personalities/WorldEditorPersonality/constants/icons';
import { NumberInput } from 'components/controls/NumberInput/NumberInput';
import { div } from 'utils/styled';
import { deleteIcon } from 'constants/icons';
import { Title } from 'components/controls/Title/Title';
import s from './PropertiesTool.module.scss';

const Header = div(s, 'header');
const Icon = div(s, 'icon');
const Name = div(s, 'name');
const Controls = div(s, 'controls');
const Block = div(s, 'block');
const LabelRow = div(s, 'label-row');
const Label = div(s, 'label');
const Control = div<{ inputs?: boolean }>(s, 'control');

export interface PatchPropertiesProps {
  mapdata: number,
  entity: number,
}

export const PatchProperties = observer(function PatchProperties(props: PatchPropertiesProps) {
  const { mapdata, entity } = props;
  const patch = WEState.map.patches[mapdata][entity];

  const [sx, sy, sz] = getScale(patch.mat);

  const unscaledMat = Array.from(patch.mat);
  applyScale(unscaledMat, [1, 1, 1]);

  const px = patch.mat[WEEntityMatrixIndex.AX];
  const py = patch.mat[WEEntityMatrixIndex.AY];
  const pz = patch.mat[WEEntityMatrixIndex.AZ];

  const [rz, rx, ry] = eulerFromMatrix(unscaledMat);

  return (
    <>
      <Header>
        <Icon>
          {patchesToolIcon}
        </Icon>
        <Name>
          <span>
            {patch.label}
          </span>
        </Name>
        <Controls>
          <Title delay={0} animated={false} fixedOn="top" title="Delete">
            {(ref) => (
              <button ref={ref} onClick={() => WEState.map.deletePatch(mapdata, entity)}>
                {deleteIcon}
              </button>
            )}
          </Title>
          <Title delay={0} animated={false} fixedOn="top" title="Focus in view">
            {(ref) => (
              <button ref={ref} onClick={() => WEState.setCam(patch.cam)}>
                <BsCameraVideo />
              </button>
            )}
          </Title>
        </Controls>
      </Header>

      <Block>
        <Label>
          {translateIcon} Position:
        </Label>
        <Control inputs>
          <NumberInput
            label="x:"
            value={px}
            onChange={(px) => WEState.map.setPatchPosition(mapdata, entity, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={py}
            onChange={(py) => WEState.map.setPatchPosition(mapdata, entity, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={pz}
            onChange={(pz) => WEState.map.setPatchPosition(mapdata, entity, px, py, pz)}
            className={s.input}
          />
        </Control>
      </Block>

      <Block>
        <Label>
          {rotateIcon}
          Rotation:
        </Label>
        <Control inputs>
          <NumberInput
            label="x:"
            value={rx}
            onChange={(rx) => WEState.map.setPatchRotation(mapdata, entity, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={ry}
            onChange={(ry) => WEState.map.setPatchRotation(mapdata, entity, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={rz}
            onChange={(rz) => WEState.map.setPatchRotation(mapdata, entity, rx, ry, rz)}
            className={s.input}
          />
        </Control>
      </Block>

      <Block>
        <LabelRow>
          <Label>
            {scaleIcon}
            Scale:
          </Label>
          <Controls>
            <Title delay={0} animated={false} fixedOn="right" title="Scale to 1,1,1">
              {(ref) => (
                <button
                  ref={ref}
                  onClick={() => WEState.map.setPatchScale(mapdata, entity, 1, 1, 1)}
                >
                  1:1
                </button>
              )}
            </Title>
          </Controls>
        </LabelRow>
        <Control inputs>
          <NumberInput
            label="x:"
            value={sx}
            onChange={(sx) => WEState.map.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={sy}
            onChange={(sy) => WEState.map.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={sz}
            onChange={(sz) => WEState.map.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
        </Control>
      </Block>
    </>
  );
});
