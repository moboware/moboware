import asyncio
from websockets import connect


async def hello(uri):
    async with connect(uri) as websocket:

        for i in range(10000):
            await websocket.send("{\"Text\":\"Hello world!\"}")
            data = await websocket.recv()
            #print("Data :{}".format(data))


asyncio.run(hello("ws://localhost:4401"))
