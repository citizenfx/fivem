import { CONTROLS, SETTINGS } from "./config";
import { RotDeg3, Vec3 } from "./math";
import { getSmartControlNormal, useKeyMapping } from "./utils";

export const CameraController = new class CameraController {
  private handle: number;

  private pos = new Vec3(0, 0, 100);
  private rot = RotDeg3.zero();

  private forwardVector = Vec3.zero();

  private move = {
    x: 0,
    y: 0,
  };

  public fov = 45;

  init() {
    this.handle = CreateCamera('DEFAULT_SCRIPTED_CAMERA', true);

    SetCamFov(this.handle, this.fov);

    this.updateCamPosition();
    this.updateCamRotation();

    RenderScriptCams(true, false, 1, false, false);

    on('setCamRot', (payload: string) => {
      try {
        [this.rot.x, this.rot.y, this.rot.z] = JSON.parse(payload);
      } catch (e) {
        console.error(e);
      }
    });
  }

  destroy() {
    if (IsCamActive(this.handle)) {
      DestroyCam(this.handle, false);
      ClearFocus();
      SetPlayerControl(PlayerId(), true, 0);

      RenderScriptCams(false, false, 0, false, false);
    }
  }

  setMoveX(x: number) {
    this.move.x += x;
  }

  setMoveY(y: number) {
    this.move.y += y;
  }

  getPosition(): Vec3 {
    return this.pos;
  }

  getForwardVector(): Vec3 {
    return this.forwardVector;
  }

  getSpeedMultiplier(): number {
    const fastNormal = getSmartControlNormal(CONTROLS.MOVE_FAST);
    const slowNormal = getSmartControlNormal(CONTROLS.MOVE_SLOW);

    const baseSpeed = SETTINGS.BASE_MOVE_MULTIPLIER;
    const fastSpeed = 1 + ((SETTINGS.FAST_MOVE_MULTIPLIER - 1) * fastNormal);
    const slowSpeed = 1 + ((SETTINGS.SLOW_MOVE_MULTIPLIER - 1) * slowNormal);

    const frameMultiplier = GetFrameTime() * 60;
    const speedMultiplier = baseSpeed * fastSpeed / slowSpeed;

    return speedMultiplier * frameMultiplier;
  }

  updatePosition(dx: number, dy: number) {
    const speedMultiplier = this.getSpeedMultiplier();

    const [forward, left] = this.rot.directions();

    this.forwardVector = forward.copy();

    forward.mult(dx * speedMultiplier);
    left.mult(dy * speedMultiplier);

    const moveVec = forward.add(left);

    this.pos.x += moveVec.x;
    this.pos.y += moveVec.y;
    this.pos.z += moveVec.z;

    this.updateCamPosition();
  }

  updateRotation(dx: number, dy: number) {
    this.rot.x += -dy * SETTINGS.LOOK_SENSETIVITY[0];
    this.rot.z += -dx * SETTINGS.LOOK_SENSETIVITY[1];

    this.rot.clamp();

    this.updateCamRotation();
  }

  update() {
    const lookX = getSmartControlNormal(CONTROLS.LOOK_X);
    const lookY = getSmartControlNormal(CONTROLS.LOOK_Y);

    const moveX = this.move.x;
    const moveY = this.move.y;

    this.updatePosition(moveX, moveY);
    this.updateRotation(lookX, lookY);
  }

  private updateCamPosition() {
    const interior = GetInteriorAtCoords(this.pos.x, this.pos.y, this.pos.z);
    LoadInterior(interior);

    SetFocusArea(this.pos.x, this.pos.y, this.pos.z, 0, 0, 0);
    SetCamCoord(this.handle, this.pos.x, this.pos.y, this.pos.z);
  }

  private updateCamRotation() {
    SetCamRot(this.handle, this.rot.x, this.rot.y, this.rot.z, 2);
  }
}

const moveFW = useKeyMapping('we_movefw', 'Move Camera forward', 'keyboard', 'w');
const moveBW = useKeyMapping('we_movebw', 'Move Camera backward', 'keyboard', 's');
const moveLB = useKeyMapping('we_movelb', 'Move Camera left', 'keyboard', 'a');
const moveRB = useKeyMapping('we_moverb', 'Move Camera right', 'keyboard', 'd');

moveFW.on(() => CameraController.setMoveX(1));
moveFW.off(() => CameraController.setMoveX(-1));

moveBW.on(() => CameraController.setMoveX(-1));
moveBW.off(() => CameraController.setMoveX(1));

moveLB.on(() => CameraController.setMoveY(-1));
moveLB.off(() => CameraController.setMoveY(1));

moveRB.on(() => CameraController.setMoveY(1));
moveRB.off(() => CameraController.setMoveY(-1));
