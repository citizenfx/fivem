import React from 'react';
import { WEEntityMatrix, WEEntityMatrixIndex, WESelectionType } from 'backend/world-editor/world-editor-types';
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
import { WEHotkeysState } from 'personalities/WorldEditorPersonality/store/WEHotkeysState';
import { WECommand } from 'personalities/WorldEditorPersonality/constants/commands';

const Header = div(s, 'header');
const Icon = div(s, 'icon');
const Name = div(s, 'name');
const Controls = div(s, 'controls');
const Block = div(s, 'block');
const LabelRow = div(s, 'label-row');
const Label = div(s, 'label');
const Control = div<{ inputs?: boolean }>(s, 'control');

function usePatch(): { mapdata: number, entity: number, label: string, mat: WEEntityMatrix } {
  if (WEState.selection.type !== WESelectionType.PATCH) {
    return {
      label: '',
      mapdata: 0,
      entity: 0,
      mat: Array(16).fill(0) as WEEntityMatrix,
    }
  }

  const { mapdata, entity, label, mat = Array(16).fill(0) as WEEntityMatrix, cam = [0,0,0,0,0,0] } = WEState.selection;

  const patch = WEState.map!.patches[mapdata]?.[entity];

  if (patch) {
    return {
      mapdata,
      entity,
      label: patch.label,
      mat: patch.mat,
    };
  }

  return {
    mapdata,
    entity,
    label,
    mat,
  };
}

export const PatchProperties = observer(function PatchProperties() {
  const { mapdata, entity, label, mat } = usePatch();

  const [sx, sy, sz] = getScale(mat);

  const unscaledMat = Array.from(mat);
  applyScale(unscaledMat, [1, 1, 1]);

  const px = mat[WEEntityMatrixIndex.AX];
  const py = mat[WEEntityMatrixIndex.AY];
  const pz = mat[WEEntityMatrixIndex.AZ];

  const [rz, rx, ry] = eulerFromMatrix(unscaledMat);

  return (
    <>
      <Header>
        <Icon>
          {patchesToolIcon}
        </Icon>
        <Name>
          <span>
            {label}
          </span>
        </Name>
        <Controls>
          <Title delay={0} animated={false} fixedOn="top" title="Delete" shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_DELETE_SELECTION)}>
            {(ref) => (
              <button ref={ref} onClick={() => WEState.map!.deletePatch(mapdata, entity)}>
                {deleteIcon}
              </button>
            )}
          </Title>
          <Title delay={0} animated={false} fixedOn="top" title="Focus in view" shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_FOCUS_SELECTION_IN_VIEW)}>
            {(ref) => (
              <button ref={ref} onClick={WEState.focusSelectionInView}>
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
            onChange={(px) => WEState.map!.setPatchPosition(mapdata, entity, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={py}
            onChange={(py) => WEState.map!.setPatchPosition(mapdata, entity, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={pz}
            onChange={(pz) => WEState.map!.setPatchPosition(mapdata, entity, px, py, pz)}
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
            onChange={(rx) => WEState.map!.setPatchRotation(mapdata, entity, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={ry}
            onChange={(ry) => WEState.map!.setPatchRotation(mapdata, entity, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={rz}
            onChange={(rz) => WEState.map!.setPatchRotation(mapdata, entity, rx, ry, rz)}
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
                  onClick={() => WEState.map!.setPatchScale(mapdata, entity, 1, 1, 1)}
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
            onChange={(sx) => WEState.map!.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={sy}
            onChange={(sy) => WEState.map!.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={sz}
            onChange={(sz) => WEState.map!.setPatchScale(mapdata, entity, sx, sy, sz)}
            className={s.input}
          />
        </Control>
      </Block>
    </>
  );
});
