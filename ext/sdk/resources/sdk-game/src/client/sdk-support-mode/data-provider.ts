import { sendSdkMessage } from "../sendSdkMessage";

setTick(() => {
  const playerPedId = PlayerPedId();

  sendSdkMessage('fxdk:data', [
    {
      key: 'player_ped_pos',
      value: GetEntityCoords(playerPedId, true),
    },
    {
      key: 'player_ped_rot',
      value: GetEntityRotation(playerPedId, 2),
    },
    {
      key: 'player_ped_heading',
      value: GetEntityHeading(playerPedId),
    },
  ]);
});
