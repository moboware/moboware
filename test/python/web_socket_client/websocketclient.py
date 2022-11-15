import asyncio
from websockets import connect


async def ReceiveData(websocket):
    try:
        print("Waiting for receive data")
        while (True):
            data = await websocket.recv()
            print("Data :{}".format(data))
    except:
        pass
    pass


async def hello(uri):
    async with connect(uri) as websocket:
        task = asyncio.create_task(ReceiveData(websocket))
        # for i in range(150000):
        # send bid
        await websocket.send("{\"Action\":\"Insert\","
                             "\"Data\": "
                             "{"
                             "\"Account\": \"mobo\","
                             "\"Instrument\": \"ABCN\","
                             "\"Price\":100500000,"
                             "\"Volume\":10,"
                             "\"IsBuy\":true,"
                             "\"Type\":\"Limit\","
                             "\"ClientId\":\"123894792187\""
                             "}"
                             "}")
        # send ask
        await websocket.send("{\"Action\":\"Insert\","
                             "\"Data\": "
                             "{"
                             "\"Account\": \"mobo\","
                             "\"Instrument\": \"ABCN\","
                             "\"Price\":100500000,"
                             "\"Volume\":10,"
                             "\"IsBuy\":false,"
                             "\"Type\":\"Limit\","
                             "\"ClientId\":\"123894792188\""
                             "}"
                             "}")
        await asyncio.sleep(5)

asyncio.run(hello("ws://localhost:4401"))
