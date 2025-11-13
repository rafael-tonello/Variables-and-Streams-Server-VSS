package vstp

import (
	"strconv"
	"strings"
	"sync"
	"time"

	"rtonello/vss/sources/controller"
	"rtonello/vss/sources/misc"
	"rtonello/vss/sources/misc/logger"
	tcpserver "rtonello/vss/sources/misc/tcpserver"
	// storage removed: VSTP must use controller API only
)

const (
	SEND_SERVER_INFO_AND_CONFS             = "serverinfo"
	PING                                   = "ping"
	PONG                                   = "pong"
	SUGGEST_NEW_CLI_ID                     = "sugestednewid"
	CHANGE_OR_CONFIRM_CLI_ID               = "setid"
	TOTAL_VARIABLES_ALREADY_BEING_OBSERVED = "aoc"
	RESPONSE_BEGIN                         = "beginresponse"
	RESPONSE_END                           = "endresponse"
	SET_VAR                                = "set"
	DELETE_VAR                             = "delete"
	DELETE_VAR_RESULT                      = "deleteresult"
	GET_VAR                                = "get"
	GET_VAR_RESPONSE                       = "varvalue"
	SUBSCRIBE_VAR                          = "subscribe"
	UNSUBSCRIBE_VAR                        = "unsubscribe"
	VAR_CHANGED                            = "varchanged"
	GET_CHILDS                             = "getchilds"
	GET_CHILDS_RESPONSE                    = "childs"
	LOCK_VAR                               = "lock"
	UNLOCK_VAR                             = "unlock"
	LOCK_VAR_RESULT                        = "lockresult"
	UNLOCK_VAR_DONE                        = "unlockdone"
	SERVER_BEGIN_HEADERS                   = "beginserverheaders"
	SERVER_END_HEADERS                     = "endserverheaders"
	HELP                                   = "help"
	SET_TELNET_SESSION                     = "telnet"
	CHECK_VAR_LOCK_STATUS                  = "lockstatus"
	CHECK_VAR_LOCK_STATUS_RESULT           = "lockstatusresult"
	ERROR                                  = "error"
)

// VSTP is a text protocol API implementation (a Go port of the C++ VSTP API).
// It implements controller.ApiInterface so it can be registered into TheController.
type VSTP struct {
	ctrl            controller.IController
	apiId           string
	clientsById     map[string]tcpserver.ITCPClient
	clientsByTcpCli map[tcpserver.ITCPClient]string
	clientsLock     sync.RWMutex

	incomingDataBuffer map[tcpserver.ITCPClient]string
	log                logger.INamedLogger
	cliIdCount         int64
}

// NewVSTP constructs and starts a VSTP server on the given port and registers
// itself on the provided controller (calls ApiStarted).
func NewVSTP(port int, ctrl controller.IController, logger logger.ILogger) (*VSTP, error) {
	v := &VSTP{
		ctrl:               ctrl,
		apiId:              "VSTPAPI",
		clientsById:        make(map[string]tcpserver.ITCPClient),
		clientsByTcpCli:    make(map[tcpserver.ITCPClient]string),
		incomingDataBuffer: make(map[tcpserver.ITCPClient]string),
		log:                logger.GetNamedLogger("Apis::VSTP"),
		cliIdCount:         0,
	}

	// create tcp server and set callbacks
	srv, err := tcpserver.New(tcpserver.WithPort(port))
	if err != nil {
		return nil, err
	}

	srv.SetOnCLientConnect(func(cli tcpserver.ITCPClient) {
		// assign unique id
		uid := misc.CreateFunName(1, 1) //strconv.FormatInt(time.Now().UnixNano(), 10)
		uid = strconv.FormatInt(v.cliIdCount, 10) + "-" + uid
		v.cliIdCount++
		// if underlying TCPClient has Tags, try set via type assertion
		if tc, ok := cli.(interface{ Tags() map[string]string }); ok {
			// ignore; Tags accessor not available - use send id message instead
			_ = tc
		}
		v.clientsLock.Lock()
		v.clientsById[uid] = cli
		v.clientsByTcpCli[cli] = uid
		v.incomingDataBuffer[cli] = ""
		v.clientsLock.Unlock()
		// send welcome info and id
		v.sentInitialHeaders(cli, uid)

		v.log.Info("New VSTP client connected, assigned id: " + uid)
	})

	srv.SetOnClientDisconnect(func(cli tcpserver.ITCPClient) {
		// remove from maps using secondary index
		v.clientsLock.Lock()
		if id, ok := v.clientsByTcpCli[cli]; ok {
			v.log.Info("VSTP client " + id + " disconnected")
			delete(v.clientsById, id)
			delete(v.clientsByTcpCli, cli)
		} else {
			// fallback: try to find by scanning (compat)
			for id, c := range v.clientsById {
				if c == cli {
					v.log.Info("VSTP client " + id + " disconnected")
					delete(v.clientsById, id)
					break
				}
			}
		}
		delete(v.incomingDataBuffer, cli)
		v.clientsLock.Unlock()
	})

	srv.SetOnClientData(func(cli tcpserver.ITCPClient, data []byte) {
		v.clientsLock.Lock()
		if _, ok := v.incomingDataBuffer[cli]; !ok {
			v.log.Warning("Received data from unknown client. The buffer will be initialized, but errors may occur because client was not properly initialized.")
			v.incomingDataBuffer[cli] = ""
			uid := "problematic " + misc.CreateFunName(1, 1)
			v.clientsLock.Lock()
			v.clientsById[uid] = cli
			v.clientsByTcpCli[cli] = uid
			v.clientsLock.Unlock()
		}

		buf := v.incomingDataBuffer[cli]
		v.clientsLock.Unlock()
		buf += string(data)
		// take lines
		for {
			if idx := strings.IndexByte(buf, '\n'); idx >= 0 {
				line := buf[:idx]
				buf = buf[idx+1:]
				line = strings.TrimRight(line, "\r")
				go v.processReceivedMessage(cli, line)
			} else {
				break
			}
		}
		v.clientsLock.Lock()
		v.incomingDataBuffer[cli] = buf
		v.clientsLock.Unlock()
	})

	// register to controller
	if ctrl != nil {
		ctrl.ApiStarted(v)
	}

	return v, nil
}

func (v *VSTP) sentInitialHeaders(cli tcpserver.ITCPClient, uid string) {
	v.protocolWrite(cli, SERVER_BEGIN_HEADERS, "")

	// minimal info: protocol and system version if controller available
	v.protocolWrite(cli, SEND_SERVER_INFO_AND_CONFS, "PROTOCOL VERSION=1.2.0")
	if v.ctrl != nil {
		v.protocolWrite(cli, SEND_SERVER_INFO_AND_CONFS, "VSS VERSION="+v.ctrl.GetSystemVersion())
	}

	v.protocolWrite(cli, SUGGEST_NEW_CLI_ID, uid)

	v.protocolWrite(cli, SERVER_END_HEADERS, "")
}

// ApiInterface implementation -------------------------------------------------
func (v *VSTP) GetApiId() string {
	return v.apiId
}

func (v *VSTP) CheckAlive(clientId string) bool {
	v.clientsLock.RLock()
	_, ok := v.clientsById[clientId]
	v.clientsLock.RUnlock()
	return ok
}

func (v *VSTP) NotifyClient(clientId string, varsAndValues []misc.Tuple[string]) bool {

	v.clientsLock.RLock()
	cli, ok := v.clientsById[clientId]
	v.clientsLock.RUnlock()
	if !ok {
		return false
	}
	// send each tuple as VAR_CHANGED
	for _, t := range varsAndValues {

		if t.Len() < 3 {
			v.log.Warning("VSTP NotifyClient: invalid tuple length for client " + clientId)
			continue
		}

		meta := t.At(1)
		metaStr := ""
		if meta != "" {
			metaStr = "(" + meta + ")"
		}
		toSend := t.At(0) + metaStr + "=" + t.At(2)

		err := v.protocolWrite(cli, VAR_CHANGED, toSend)
		if err != nil {
			v.log.Error("Failed to send VAR_CHANGED to client:", err)
			return false
		}
	}
	return true
}

// Helpers ---------------------------------------------------------------------
func (v *VSTP) protocolWrite(cli tcpserver.ITCPClient, cmd, data string) error {
	buf := cmd + ":" + data + "\n"
	//cliId := v.clientsByTcpCli[cli]
	//v.log.Debug("VSTP protocolWrite sending to " + cliId + ": " + buf)
	//fmt.Println("VSTP protocolWrite sending to", cliId, ":", buf)
	return cli.SendString(buf)
}

// processReceivedMessage parses and executes the protocol command
func (v *VSTP) processReceivedMessage(cli tcpserver.ITCPClient, message string) {
	// message form: command:payload
	cmd, payload := separateKeyAndValue(message)
	cmd = strings.TrimSpace(cmd)
	payload = strings.TrimSpace(payload)

	switch cmd {
	case SET_VAR:
		// payload: var=val
		name, val := separateKeyAndValue(payload)
		if name != "" {
			// use controller to set the variable and wait for result
			if v.ctrl != nil {
				err := <-v.ctrl.SetVar(name, misc.NewDynamicVar(val))
				v.protocolWrite(cli, RESPONSE_BEGIN, SET_VAR+":"+payload)
				if err != nil {
					v.protocolWrite(cli, ERROR, "set error: "+err.Error())
				}
				v.protocolWrite(cli, RESPONSE_END, SET_VAR+":"+payload)
			}
		} else {
			v.protocolWrite(cli, RESPONSE_BEGIN, SET_VAR+":"+payload)
			v.protocolWrite(cli, ERROR, "set error: invalid variable name")
			v.protocolWrite(cli, RESPONSE_END, SET_VAR+":"+payload)
		}
	case DELETE_VAR:
		name := payload
		if v.ctrl != nil {
			err := <-v.ctrl.DelVar(name)
			v.protocolWrite(cli, RESPONSE_BEGIN, DELETE_VAR+":"+payload)
			if err != nil {
				v.protocolWrite(cli, DELETE_VAR_RESULT, name+"=failure:"+err.Error())
			} else {
				v.protocolWrite(cli, DELETE_VAR_RESULT, name+"=success")
			}
			v.protocolWrite(cli, RESPONSE_END, DELETE_VAR+":"+payload)
		}
	case GET_VAR:
		name := payload
		// read value(s) and reply
		if v.ctrl != nil {
			res := <-v.ctrl.GetVars(name, misc.NewDynamicVar(""))
			v.protocolWrite(cli, RESPONSE_BEGIN, GET_VAR+":"+name)
			if res.Err != nil {
				v.protocolWrite(cli, ERROR, "get error: "+res.Err.Error())
			} else {
				for _, tup := range res.Values {
					nameDV := tup.At(0)
					valDV := tup.At(1)
					nameStr := (&nameDV).GetString()
					valStr := (&valDV).GetString()
					v.protocolWrite(cli, GET_VAR_RESPONSE, nameStr+"="+valStr)
				}
			}
			v.protocolWrite(cli, RESPONSE_END, GET_VAR+":"+name)
		}
	case SUBSCRIBE_VAR:
		// payload may contain metadata like name(meta)
		name, meta := separateNameAndMetadata(payload)
		// register observation on controller
		if v.ctrl != nil {
			// we need a client id: use the secondary index for O(1) lookup
			v.clientsLock.Lock()
			cid, ok := v.clientsByTcpCli[cli]
			if !ok || cid == "" {
				cid = strconv.FormatInt(time.Now().UnixNano(), 10)
				v.clientsById[cid] = cli
				v.clientsByTcpCli[cli] = cid
			}
			v.clientsLock.Unlock()
			v.protocolWrite(cli, RESPONSE_BEGIN, SUBSCRIBE_VAR+":"+payload)
			v.ctrl.ObserveVar(name, cid, meta, v)
			v.protocolWrite(cli, RESPONSE_END, SUBSCRIBE_VAR+":"+payload)
			v.log.Info("Client subscribed to variable: " + name + " (metadata: " + meta + ")")
		}
	case UNSUBSCRIBE_VAR:
		name, meta := separateNameAndMetadata(payload)
		if v.ctrl != nil {
			// find client id via secondary index
			v.clientsLock.RLock()
			cid, ok := v.clientsByTcpCli[cli]
			v.clientsLock.RUnlock()
			if ok && cid != "" {
				v.protocolWrite(cli, RESPONSE_BEGIN, UNSUBSCRIBE_VAR+":"+payload)
				v.ctrl.StopObservingVar(name, cid, meta, v)
				v.protocolWrite(cli, RESPONSE_END, UNSUBSCRIBE_VAR+":"+payload)
			}
		}
	case LOCK_VAR:
		// payload: var[=timeout]
		name, val := separateKeyAndValue(payload)
		timeout := uint(^uint(0))
		if val != "" {
			if t, err := strconv.Atoi(val); err == nil {
				timeout = uint(t)
			}
		}
		if v.ctrl != nil {
			err := <-v.ctrl.LockVar(name, timeout)
			v.protocolWrite(cli, RESPONSE_BEGIN, LOCK_VAR+":"+payload)
			if err != nil {
				v.protocolWrite(cli, LOCK_VAR_RESULT, name+"=failure:"+err.Error())
			} else {
				v.protocolWrite(cli, LOCK_VAR_RESULT, name+"=success")
			}
		} else {
			v.protocolWrite(cli, RESPONSE_BEGIN, LOCK_VAR+":"+payload)
			v.protocolWrite(cli, LOCK_VAR_RESULT, name+"=success")
		}
		v.protocolWrite(cli, RESPONSE_END, LOCK_VAR+":"+payload)
	case UNLOCK_VAR:
		name, _ := separateKeyAndValue(payload)
		if v.ctrl != nil {
			<-v.ctrl.UnlockVar(name)
		}
		v.protocolWrite(cli, RESPONSE_BEGIN, UNLOCK_VAR+":"+payload)
		v.protocolWrite(cli, UNLOCK_VAR_DONE, name)
		v.protocolWrite(cli, RESPONSE_END, UNLOCK_VAR+":"+payload)
	case PING:
		v.protocolWrite(cli, RESPONSE_BEGIN, PING+":"+payload)
		v.protocolWrite(cli, PONG, "")
		v.protocolWrite(cli, RESPONSE_END, PING+":"+payload)

	// client sends its requested id to resume a previous session
	case CHANGE_OR_CONFIRM_CLI_ID:
		// payload is the requested id
		newId := payload
		// update mapping: remove old id pointing to this cli and set newId->cli
		{
			// get oldId via secondary index and update both maps
			v.clientsLock.Lock()
			oldId := v.clientsByTcpCli[cli]
			if oldId != "" {
				delete(v.clientsById, oldId)
			}

			v.log.Info("Client requested id change: " + oldId + " -> " + newId)
			v.clientsById[newId] = cli
			v.clientsByTcpCli[cli] = newId
			v.clientsLock.Unlock()
		}
		// notify controller that client connected (so it can re-send observing vars)
		if v.ctrl != nil {
			_, observing := v.ctrl.ClientConnected(newId, v)
			// send total observed count
			v.protocolWrite(cli, RESPONSE_BEGIN, CHANGE_OR_CONFIRM_CLI_ID+":"+payload)
			v.protocolWrite(cli, TOTAL_VARIABLES_ALREADY_BEING_OBSERVED, strconv.Itoa(observing))
			v.protocolWrite(cli, RESPONSE_END, CHANGE_OR_CONFIRM_CLI_ID+":"+payload)
		}

	case GET_CHILDS:
		// payload is parent var
		parent := payload
		if v.ctrl != nil {
			childs := <-v.ctrl.GetChildsOfVar(parent)
			// join by comma
			resp := strings.Join(childs, ",")
			v.protocolWrite(cli, RESPONSE_BEGIN, GET_CHILDS+":"+payload)
			v.protocolWrite(cli, GET_CHILDS_RESPONSE, resp)
			v.protocolWrite(cli, RESPONSE_END, GET_CHILDS+":"+payload)
		}

	case CHECK_VAR_LOCK_STATUS:
		name := payload
		if v.ctrl != nil {
			locked := <-v.ctrl.IsVarLocked(name)
			status := "unlocked"
			if locked {
				status = "locked"
			}
			v.protocolWrite(cli, CHECK_VAR_LOCK_STATUS_RESULT, name+"="+status)
		}
	default:
		// ignore unknown
		v.log.Info("Unknown VSTP command received: " + cmd)
	}
}

// small helpers ---------------------------------------------------------------
func separateKeyAndValue(s string) (string, string) {
	for i := 0; i < len(s); i++ {
		if strings.ContainsAny(string(s[i]), "=;:") {
			return s[:i], s[i+1:]
		}
	}
	return s, ""
}

func separateNameAndMetadata(original string) (string, string) {
	if p := strings.IndexByte(original, '('); p >= 0 {
		name := original[:p]
		meta := original[p+1:]
		meta = strings.TrimSuffix(meta, ")")
		return name, meta
	}
	return original, ""
}
