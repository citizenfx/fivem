import React from 'react';
import { WEEntityMatrixIndex } from 'backend/world-editor/world-editor-types';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { applyScale, eulerFromMatrix, getScale } from 'shared/math';
import { BsCameraVideo, BsChevronBarDown } from 'react-icons/bs';
import { additionsToolIcon, rotateIcon, scaleIcon, translateIcon } from 'personalities/WorldEditorPersonality/constants/icons';
import { NumberInput } from 'components/controls/NumberInput/NumberInput';
import { div } from 'utils/styled';
import { deleteIcon } from 'constants/icons';
import { Title } from 'components/controls/Title/Title';
import { WEHotkeysState } from 'personalities/WorldEditorPersonality/store/WEHotkeysState';
import { WECommand } from 'personalities/WorldEditorPersonality/constants/commands';
import s from './PropertiesTool.module.scss';

const Header = div(s, 'header');
const Icon = div(s, 'icon');
const Name = div(s, 'name');
const Controls = div(s, 'controls');
const Block = div(s, 'block');
const LabelRow = div(s, 'label-row');
const Label = div(s, 'label');
const Control = div<{ inputs?: boolean }>(s, 'control');

export interface AdditionProprtiesProps {
  additionId: string,
}

export const AdditionProprties = observer(function AdditionProprties(props: AdditionProprtiesProps) {
  const { additionId } = props;
  const addition = WEState.map!.additions[additionId];

  const [sx, sy, sz] = getScale(addition.mat);

  const unscaledMat = Array.from(addition.mat);
  applyScale(unscaledMat, [1, 1, 1]);

  const px = addition.mat[WEEntityMatrixIndex.AX];
  const py = addition.mat[WEEntityMatrixIndex.AY];
  const pz = addition.mat[WEEntityMatrixIndex.AZ];

  const [rz, rx, ry] = eulerFromMatrix(unscaledMat);

  const handleFocusInView = () => {
    WEState.focusInView(addition.cam, [px, py, pz]);
  };

  return (
    <>
      <Header>
        <Icon>
          {additionsToolIcon}
        </Icon>
        <Name>
          <span>
            {addition.label}
          </span>
        </Name>
        <Controls>
          <Title delay={0} animated={false} fixedOn="top" title="Delete">
            {(ref) => (
              <button ref={ref} onClick={() => WEState.map!.deleteAddition(additionId)}>
                {deleteIcon}
              </button>
            )}
          </Title>
          <Title delay={0} animated={false} fixedOn="top" title="Focus in view">
            {(ref) => (
              <button ref={ref} onClick={handleFocusInView}>
                <BsCameraVideo />
              </button>
            )}
          </Title>
        </Controls>
      </Header>

      <Block>
        <LabelRow>
          <Label>
            {translateIcon} Position:
          </Label>
          <Controls>
            <Title
              delay={0}
              animated={false}
              fixedOn="right"
              title="Set on ground"
              shortcut={WEHotkeysState.getCommandHotkey(WECommand.ACTION_SET_ADDITION_ON_GROUND)}
            >
              {(ref) => (
                <button
                  ref={ref}
                  onClick={() => WEState.map!.setAdditionOnGround(additionId)}
                >
                  <BsChevronBarDown />
                </button>
              )}
            </Title>
          </Controls>
        </LabelRow>
        <Control inputs>
          <NumberInput
            label="x:"
            value={px}
            onChange={(px) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={py}
            onChange={(py) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={pz}
            onChange={(pz) => WEState.map!.setAdditionPosition(additionId, px, py, pz)}
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
            onChange={(rx) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={ry}
            onChange={(ry) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={rz}
            onChange={(rz) => WEState.map!.setAdditionRotation(additionId, rx, ry, rz)}
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
                  onClick={() => WEState.map!.setAdditionScale(additionId, 1, 1, 1)}
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
            onChange={(sx) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="y:"
            value={sy}
            onChange={(sy) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
            className={s.input}
          />
          <NumberInput
            label="z:"
            value={sz}
            onChange={(sz) => WEState.map!.setAdditionScale(additionId, sx, sy, sz)}
            className={s.input}
          />
        </Control>
      </Block>
    </>
  );
});
