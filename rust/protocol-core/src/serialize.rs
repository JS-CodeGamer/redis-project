use crate::RedisProto;

impl From<RedisProto> for String {
    fn from(value: RedisProto) -> Self {
        match value {
            RedisProto::SimpleString(x) => format!("+{x}\r\n"),
            RedisProto::SimpleErr(x) => format!("-{x}\r\n"),
            RedisProto::Int(x) => format!(":{x}\r\n"),
            RedisProto::BulkString(x) => format!("${}\r\n{x}\r\n", x.len()),
            RedisProto::Array(x) => {
                format!(
                    "*{}\r\n{}\r\n",
                    x.len(),
                    x.iter()
                        .map(|x| x.clone().into())
                        .collect::<Vec<String>>()
                        .join("\r\n")
                )
            }
            RedisProto::Bool(x) => format!("#{}\r\n", if x { 't' } else { 'f' }),
            RedisProto::Double(x) => format!(",{x}\r\n"),
            RedisProto::BigInt(x) => format!("({x}\r\n"),
            RedisProto::BulkErr(x) => format!("!{}\r\n{x}\r\n", x.len()),
            RedisProto::Null => format!("_\r\n"),
        }
    }
}
