import asyncio
from websockets import connect

orderBook = []

async def ReceiveData(websocket):
  try:
      print("Waiting for receive data")
      while (True):
          data = await websocket.recv()
          print("Data :{}".format(data))
          #orderReplyData = data["OrderReply"]
          #clientId = data["OrderReply"]["ClientId"]
          #id = data["OrderInsert"]["Id"]
          #orderBook[clientId] = id
  except:
      pass
  pass


async def hello(uri):
  async with connect(uri) as websocket:
      task = asyncio.create_task(ReceiveData(websocket))

      for i in range(250):

        i = 123894792180

        # for i in range(150000):
        # send bid
        i = i + 1
        bidOrder1 = "{\"Action\":\"Insert\",\"Data\":{\"Account\": \"mobo\",\"Instrument\": \"ABCN\",\"Price\":100500000,\"Volume\":10,\"IsBuy\":true,\"Type\":\"Limit\",\"ClientId\":\"" + "{}".format(i) + "\"}}"
        await websocket.send(bidOrder1)

        i = i + 1
        bidOrder2 = "{\"Action\":\"Insert\",\"Data\":{\"Account\": \"mobo\",\"Instrument\": \"ABCN\",\"Price\":100500000,\"Volume\":25,\"IsBuy\":true,\"Type\":\"Limit\",\"ClientId\":\"" + "{}".format(i) + "\"}}"
        await websocket.send(bidOrder2)

        # send ask
        i = i + 1
        askOrder1 = "{\"Action\":\"Insert\",\"Data\":{\"Account\": \"mobo\",\"Instrument\": \"ABCN\",\"Price\":100700000,\"Volume\":10,\"IsBuy\":false,\"Type\":\"Limit\",\"ClientId\":\"" + "{}".format(i) + "\"}}"
        await websocket.send(askOrder1)

        i = i + 1
        askOrder2 = "{\"Action\":\"Insert\",\"Data\":{\"Account\": \"mobo\",\"Instrument\": \"ABCN\",\"Price\":100500000,\"Volume\":55,\"IsBuy\":false,\"Type\":\"Limit\",\"ClientId\":\"" + "{}".format(i) + "\"}}"
        await websocket.send(askOrder2)


        await websocket.send("{\"Action\":\"GetBook\", \"Data\":{\"Account\": \"mobo\",\"Instrument\": \"ABCN\"}}")
        pass
      await asyncio.sleep(1)


asyncio.run(hello("ws://localhost:4401"))
