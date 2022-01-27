
/* Default Type */
static int DefaultTypeCB(NuSocketTypeNode_t *TypeNode)
{
    return NuSocketTypeCBPass;
}

static int DefaultTypeCB_NoProtocolCB(NuSocketTypeNode_t *TypeNode)
{
    return NuSocketTypeCBDone;
}

static int DefaultOnEventCB(NuSocketTypeNode_t *TypeNode)
{
    if(TypeNode->ByteLeftForRecv <= 0)
    {
        ioctl(TypeNode->InFD, FIONREAD, &(TypeNode->ByteLeftForRecv));
    }

    return (TypeNode->ByteLeftForRecv > 0) ? NuSocketTypeCBPass: NuSocketTypeCBError;
}

static int DefaultSend(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    int WriteLen = NuSocketTypeCBError;
    int FD = TypeNode->OutFD;

    if(FD > 0)
    {
        do
        {
            if((WriteLen = write(FD, Msg, MsgLen)) <= 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    WriteLen = NuSocketTypeCBPass;
                }
                else
                {
                    WriteLen = NuSocketTypeCBError;
                }
            }
        }
        while(0);
    }

    return WriteLen;
}

static int DefaultRecv(NuSocketTypeNode_t *TypeNode, void *Msg, size_t MsgLen)
{
    int     ReadLen = NuSocketTypeCBError;
    int     FD = TypeNode->InFD;

    if(FD > 0)
    {
        do
        {
            if((ReadLen = read(FD, Msg, MsgLen)) < 0)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                else if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    ReadLen = NuSocketTypeCBPass;
                }
                else
                {
                    ReadLen = NuSocketTypeCBError;
                }
            }
            else if(ReadLen == 0)
            {
                ReadLen = NuSocketTypeCBError;
            }
        }
        while(0);
    }

    return ReadLen;
}

static int DefaultSendV(NuSocketTypeNode_t *TypeNode, struct iovec *iov, int iovCnt)
{
    int FD = TypeNode->OutFD;

	int sendn = 0;
	int len = 0, rtn = 0;
	struct iovec *v = iov;
	int    n = iovCnt;

    if(FD <= 0)
    {
		return NuSocketTypeCBError;
	}

SEND:
	do 
	{
		sendn = writev(FD, (const struct iovec *)v, n);
	} while (sendn == -1 && errno == EINTR);

	if (sendn < 0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK)
		{
			goto SEND;
		}
		else
		{   /* fail */
			rtn = sendn;
			goto EXIT;
		}
	}
	else
	{
		len = sendn;
		while (len > 0 && n > 0)
		{
			if (len < v->iov_len)
			{
				v->iov_len -= len;
				v->iov_base += len;
				rtn += len;
				break;
			}
			else
			{   /* send a slice complete */
				len -= v->iov_len;
				rtn += v->iov_len;

				v->iov_len = 0;

				--n;
				++v;
			}
		}
		if (n > 0)
		{
			goto SEND;
		}
	}

EXIT:
    return rtn;
}

static int DefaultReadV(NuSocketTypeNode_t *TypeNode, struct iovec *iov, int iovCnt)
{
    int     FD = TypeNode->InFD;
	return readv(FD, (const struct iovec *)iov, iovCnt);
}


static NuSocketType_t DefaultNullType = {"DefaultNullType", 0, &DefaultTypeCB, &DefaultTypeCB, &DefaultOnEventCB, &DefaultSend, &DefaultRecv, &DefaultSendV, &DefaultReadV};

static NuSocketType_t *TypeCompletion(NuSocketType_t *Type)
{
    if(!Type)
    {
        Type = &DefaultNullType;
    }
    else
    {
        if(!(Type->SetOnline))
        {
            if(Type->Property & NuSocketTypeProperty_NoOnConnect)
            {
                Type->SetOnline = &DefaultTypeCB_NoProtocolCB;
            }
            else
            {
                Type->SetOnline = &DefaultTypeCB;
            }
        }

        if(!(Type->SetOffline))
        {
            if(Type->Property & NuSocketTypeProperty_NoOnDisconnect)
            {
                Type->SetOffline = &DefaultTypeCB_NoProtocolCB;
            }
            else
            {
                Type->SetOffline = &DefaultTypeCB;
            }
        }
        
        if(!(Type->OnEvent))
        {
            Type->OnEvent = &DefaultOnEventCB;
        }
 
        if(!(Type->Send))
        {
            Type->Send = &DefaultSend;
        }

        if(!(Type->Recv))
        {
            Type->Recv = &DefaultRecv;
        }

		if (!(Type->SendV))
		{
            Type->SendV = &DefaultSendV;
		}

		if (!(Type->ReadV))
		{
            Type->ReadV = &DefaultReadV;
		}
    }

    return Type;
}


/* Default Protocol */
static void DefaultProtocolCB(NuSocketNode_t *Node, void *Argu)
{
    return;
}

static NuSocketProtocol_t DefaultNullProtocol = {&DefaultProtocolCB, &DefaultProtocolCB, &DefaultProtocolCB, &DefaultProtocolCB, &DefaultProtocolCB};

static NuSocketProtocol_t *ProtocolCompletion(NuSocketProtocol_t *Protocol)
{
    if(!Protocol)
    {
        Protocol = &DefaultNullProtocol;
    }
    else
    {
        if(!(Protocol->OnConnect))
        {
            Protocol->OnConnect = &DefaultProtocolCB;
        }

        if(!(Protocol->OnDataArrive))
        {
            Protocol->OnDataArrive = &DefaultProtocolCB;
        }

        if(!(Protocol->OnRemoteTimeout))
        {
            Protocol->OnRemoteTimeout = &DefaultProtocolCB;
        }

        if(!(Protocol->OnLocalTimeout))
        {
            Protocol->OnLocalTimeout = &DefaultProtocolCB;
        }

        if(!(Protocol->OnDisconnect))
        {
            Protocol->OnDisconnect = &DefaultProtocolCB;
        }
    }

    return Protocol;
}

/* Defailt LogCB */
static void DefaultLogCB(char *Format, va_list ArguList, void *Argu)
{
    return;
}

