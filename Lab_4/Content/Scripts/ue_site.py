import sys
import unreal_engine as ue


class UnrealEngineOutput:
    def __init__(self, logger):
        self.logger = logger

    def write(self, buf):
        self.logger(buf)

    def flush(self):
        return


sys.stdout = UnrealEngineOutput(ue.log)
sys.stderr = UnrealEngineOutput(ue.log_error)
