import { WEApi } from "@sdk-root/backend/world-editor/world-editor-game-api";
import { CONTROLS, SETTINGS } from "./config";
import { limitPrecision, rad2deg, rotation, RotDeg3, Vec3 } from "./math";
import { SettingsManager } from "./settings-manager";
import { getSmartControlNormal, onWEApi, useKeyMapping } from "./utils";

export const CameraManager = new class CameraManager {
  private handle: number;
  private destroyed = false;

  private pos = new Vec3(0, 0, 100);
  private rot = RotDeg3.zero();

  private forwardVector = Vec3.zero();

  private move = {
    x: 0,
    y: 0,
  };

  public baseMoveMultiplier = SETTINGS.BASE_MOVE_MULTIPLIER;
  public fov = 45;

  preinit() {
    onWEApi(WEApi.SetCamBaseMultiplier, (multiplier) => {
      this.baseMoveMultiplier = multiplier;
    });
  }

  init(ease = false, easeTime = 1) {
    this.destroyed = false;

    const ped = PlayerPedId();
    SetEntityVisible(ped, false, false);
    FreezeEntityPosition(ped, true);
    SetPlayerControl(PlayerId(), false, 0);

    this.handle = CreateCamera('DEFAULT_SCRIPTED_CAMERA', true);

    SetCamFov(this.handle, this.fov);

    this.updateCamPosition();
    this.updateCamRotation();

    RenderScriptCams(true, ease, easeTime, false, false);
  }

  destroy(ease = false, easeTime = 0, invincible = false, shouldClearFocus = true) {
    if (this.destroyed) {
      return;
    }

    this.destroyed = true;

    SetPlayerControl(PlayerId(), true, 0);
    const ped = PlayerPedId();
    if (!IsEntityVisible(ped)) {
      SetEntityVisible(ped, true, false);
    }

    if (!IsPedInAnyVehicle(ped, false)) {
      SetEntityCollision(ped, true, false);
    }

    FreezeEntityPosition(ped, false);
    SetPlayerInvincible(PlayerId(), invincible);
    ClearPedTasksImmediately(ped);

    if (shouldClearFocus) {
      ClearFocus();
    }

    RenderScriptCams(false, ease, easeTime, false, false);

    DestroyCam(this.handle, false);
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

  getCam(): [number, number, number, number, number, number] {
    return [
      this.pos.x, this.pos.y, this.pos.z,
      this.rot.x, this.rot.y, this.rot.z,
    ];
  }

  getCamLimitedPrecision(): [number, number, number, number, number, number] {
    return limitPrecision(this.getCam(), 10000) as any;
  }

  setCam([px, py, pz, rx, ry, rz]) {
    this.pos.x = px;
    this.pos.y = py;
    this.pos.z = pz;

    this.rot.x = rx;
    this.rot.y = ry;
    this.rot.z = rz;
  }

  setLookAt(x: number, y: number, z: number) {
    this.rot.z = rotation(
      [this.pos.x, this.pos.y],
      [x, y],
    );
    this.rot.x = 0;

    const fw = this.rot.forward();

    const dir = new Vec3(
      x - this.pos.x,
      y - this.pos.y,
      z - this.pos.z,
    );

    this.rot.x = rad2deg(Math.acos(fw.dot(dir) / (fw.length() * dir.length())));

    if (z < this.pos.z) {
      this.rot.x *= -1;
    }

    this.rot.clamp();
  }

  getForwardVector(): Vec3 {
    return this.forwardVector;
  }

  getSpeedMultiplier(): number {
    const fastNormal = getSmartControlNormal(CONTROLS.MOVE_FAST);
    const slowNormal = getSmartControlNormal(CONTROLS.MOVE_SLOW);

    const baseSpeed = this.baseMoveMultiplier;
    const fastSpeed = 1 + ((SETTINGS.FAST_MOVE_MULTIPLIER - 1) * fastNormal);
    const slowSpeed = 1 + ((SETTINGS.SLOW_MOVE_MULTIPLIER - 1) * slowNormal);

    const frameMultiplier = GetFrameTime() * 60;
    const speedMultiplier = baseSpeed * fastSpeed / slowSpeed;

    return speedMultiplier * frameMultiplier;
  }

  acceleration = 0;
  speedy = 150; // ms
  updatePosition() {
    const dx = this.move.x;
    const dy = this.move.y;

    if (!dx && !dy) {
      this.acceleration = 0;
    } else {
      if (this.acceleration < this.speedy) {
        this.acceleration += GetFrameTime() * 1000;

        if (this.acceleration > this.speedy) {
          this.acceleration = this.speedy;
        }
      }
    }

    const speedMultiplier = this.getSpeedMultiplier();

    const [forward, left] = this.rot.directions();

    this.forwardVector = forward.copy();
    const acceleration = this.acceleration / this.speedy;

    forward.mult((dx * speedMultiplier) * acceleration);
    left.mult((dy * speedMultiplier) * acceleration);

    const moveVec = forward.add(left);

    this.pos.x += moveVec.x;
    this.pos.y += moveVec.y;
    this.pos.z += moveVec.z;

    if (SettingsManager.settings.cameraAboveTheGround) {
      const [success, groundZ] = GetGroundZFor_3dCoord(this.pos.x, this.pos.y, this.pos.z - moveVec.z + .5, false);
      if (success && this.pos.z < groundZ) {
        this.pos.z = groundZ + .5;
      }
    }

    this.updateCamPosition();
  }

  updateRotation() {
    const dx = getSmartControlNormal(CONTROLS.LOOK_X) * 10;
    const dy = getSmartControlNormal(CONTROLS.LOOK_Y) * 10;

    this.rot.x += -dy * SettingsManager.settings.mouseSensetivity/100;
    this.rot.z += -dx * SettingsManager.settings.mouseSensetivity/100;

    this.rot.clamp();

    this.updateCamRotation();
  }

  update() {
    if (this.destroyed) {
      return;
    }

    this.updatePosition();
    this.updateRotation();
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

moveFW.on(() => CameraManager.setMoveX(1));
moveFW.off(() => CameraManager.setMoveX(-1));

moveBW.on(() => CameraManager.setMoveX(-1));
moveBW.off(() => CameraManager.setMoveX(1));

moveLB.on(() => CameraManager.setMoveY(-1));
moveLB.off(() => CameraManager.setMoveY(1));

moveRB.on(() => CameraManager.setMoveY(1));
moveRB.off(() => CameraManager.setMoveY(-1));
