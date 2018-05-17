extern "C"{
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_string.h>
#include <stdio.h>

static char* ngx_http_tcpservlet(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);
static ngx_int_t ngx_http_tcpservlet_handler(ngx_http_request_t* r);
static void ngx_http_tcpservlet_body_handler(ngx_http_request_t* r);
}

#include <string>
using namespace std;
#include "service_request.h"

static ServiceRequest service;

static ngx_command_t ngx_http_tcpservlet_commands[] = {
    {
        ngx_string("configure_server"),
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE2,
        ngx_http_tcpservlet,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL
    },
    ngx_null_command
};

static char* ngx_http_tcpservlet(ngx_conf_t* cf, ngx_command_t* cmd, void* conf)
{
    ngx_http_core_loc_conf_t *clcf;
    clcf = (ngx_http_core_loc_conf_t*)ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_tcpservlet_handler;
    if(cf->args->nelts < 3)
    {
        return (char*)NGX_CONF_ERROR;
    }
    string ip, port;
    ngx_str_t* serv_arg = (ngx_str_t*)(cf->args->elts);
    serv_arg++;
    ip = string((char*)(serv_arg->data), serv_arg->len);
    serv_arg++;
    port = string((char*)(serv_arg->data), serv_arg->len);

    service.SetAddr(ip, port);
    return NGX_CONF_OK;
}

static ngx_http_module_t ngx_http_tcpservlet_module_ctx = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

ngx_module_t ngx_http_tcpservlet_module = {
    NGX_MODULE_V1,
    &ngx_http_tcpservlet_module_ctx,
    ngx_http_tcpservlet_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_tcpservlet_handler(ngx_http_request_t* r)
{
    if(!(r->method & NGX_HTTP_POST))
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Not post, nginx http not allowed.");
        return NGX_HTTP_NOT_ALLOWED;
    }
    ngx_int_t rc = ngx_http_read_client_request_body(r, ngx_http_tcpservlet_body_handler);
    if(rc >= NGX_HTTP_SPECIAL_RESPONSE)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_http_tcpservlet_handler set parse body handler error.");
        return rc;
    }
    return NGX_DONE;
}

void ngx_http_tcpservlet_body_handler(ngx_http_request_t* r)
{
    ngx_str_t response;
    do
    {
        string tmp_body;
        int tmp_length = 0;
        ngx_str_t body = ngx_null_string;
        ngx_http_request_body_t* rb = r->request_body;
        if(rb && rb->bufs)
        {
            ngx_chain_t* chain_buf = rb->bufs;
            while(chain_buf)
            {
                //body.data = (u_char *)rb->bufs->buf->pos;
                //body.len = rb->bufs->buf->last - rb->bufs->buf->pos;
                body.data = (u_char *)chain_buf->buf->pos;
                body.len = chain_buf->buf->last - chain_buf->buf->pos;
                tmp_body += string((char*)body.data, body.len);
                tmp_length += body.len;
                chain_buf = chain_buf->next;
            }
        }
        else
        {
            ngx_log_error(NGX_LOG_WARN, r->connection->log, 0, "request is null or request body is empty.");
            r->headers_out.status = NGX_HTTP_BAD_REQUEST;
            ngx_str_set(&response, "request is empty or request body is null.");
            break;
        }

        service.SetLog(r->connection->log);
        PARAM_INFO param_info;
        if(service.ParseMsgBody(tmp_body.c_str(), tmp_length, param_info) < 0)
        {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "parse message body failed.");
            r->headers_out.status = NGX_HTTP_BAD_REQUEST;
            ngx_str_set(&response, "parse message body failed.");
            break;
        }
        string result = service.GetdataTC(param_info);
        if(result.empty())
        {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "thrift rpc failed, maxcode=%s, mincode=%s", param_info.maxcode_.c_str(), param_info.mincode_.c_str());
            r->headers_out.status = NGX_HTTP_BAD_REQUEST;
            ngx_str_set(&response, "thrift rpc failed.");
            break;
        }

        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0, "GetdataTC result: %s", result.c_str());

        response.len = result.length();
        /*response.data = (u_char*)ngx_pcalloc(r->pool, response.len);
        ngx_memcpy(response.data, result.c_str(), response.len);*/
        response.data = (u_char*)result.c_str();
        r->headers_out.status = NGX_HTTP_OK;
    }while(0);
    
    ngx_str_t type = ngx_string("text/plain");
    r->headers_out.content_length_n = response.len;
    r->headers_out.content_type = type;
    ngx_int_t rc = ngx_http_send_header(r);
    if(rc == NGX_ERROR || rc > NGX_OK || r->header_only)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_http_send_header failed, errno=%i", rc);
        return;
    }

    ngx_buf_t* b = ngx_create_temp_buf(r->pool, response.len);
    if(b == NULL)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ngx_create_temp_buf failed.");
        return;
    }
    ngx_memcpy(b->pos, response.data, response.len);
    b->last = b->pos + response.len;
    b->last_buf = 1;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;
    rc = ngx_http_output_filter(r, &out);
    (void)ngx_http_finalize_request(r, rc);
}

