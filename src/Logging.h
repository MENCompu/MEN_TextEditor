typedef enum {
    Log_Null,
    Log_MemoryInitialization,
    Log_NormalMode,
    Log_InsertMode,
    Log_VisualMode,
    Log_TerminalMode,
    Log_CommandLineMode,
    Log_MoveCursorHorByChar,
    Log_MoveCursorVertByChar,
    Log_MoveCursorByWord,
    Log_MoveCursorByBlankSpace,
    Log_InsertingText,
    Log_DeletingLine,
    Log_YankingLine,
    Log_PastingLine,
    Log_NewLine,
    Log_CreatingSubWindow,
    Log_DeletingSubWindow,
    Log_MoveThroughtSubWindow,
    Log_CreatingBufferFromFile,
} LogID;

typedef struct {
    StringBuffer *logBuffer;

    char *lastLog;

    char *beginLastLogEntry;
    LogID lastLogEntryID;
    u32 lastLogRepeats;
} LogSystem;

GlobalVariable LogSystem logSystem;

#define Logging(id, string, time) \
    char array[] = string;    \
    Logging_(id, array, ARRAYLENGTH(string) - 1, time);

void Logging_(LogID logID, char *string, u32 length, f32 time) {
    StringBuffer *logBuffer = logSystem.logBuffer;

    b32 repeatedLog = logID == logSystem.lastLogEntryID;

    char *logString = (repeatedLog) ? logSystem.beginLastLogEntry 
	                            : logBuffer->buffer + logBuffer->usedSize;

    CopyStringForward(logString, string, length);

    char *newEndString = ProcessString(logString, length, time, 0);

    if (repeatedLog) {
        ++logSystem.lastLogRepeats;

	char countString[] = " [repetitions: %d].";
	
        CopyStringForward(newEndString + 1, countString, ARRAYLENGTH(countString) - 1);
        newEndString = ProcessString(newEndString + 1, ARRAYLENGTH(countString) - 1, time, 
			             logSystem.lastLogRepeats);
    } else {
        logSystem.beginLastLogEntry = logString;
        logSystem.lastLogRepeats = 0;
	logSystem.lastLogEntryID = logID;
    }

    *++newEndString = '\r';
    *++newEndString = '\n';

    logBuffer->usedSize = (u32)( newEndString + 1 - logBuffer->buffer );
}
