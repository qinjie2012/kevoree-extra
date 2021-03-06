/*
 * HA-JDBC: High-Availability JDBC
 * Copyright 2004-2009 Paul Ferraro
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package net.sf.hajdbc.cache.lazy;

import net.sf.hajdbc.Database;
import net.sf.hajdbc.DatabaseCluster;
import net.sf.hajdbc.cache.AbstractDatabaseMetaDataCacheFactory;
import net.sf.hajdbc.cache.DatabaseMetaDataCache;
import net.sf.hajdbc.cache.DatabaseMetaDataSupportFactory;

/**
 * @author Paul Ferraro
 *
 */
public class SharedLazyDatabaseMetaDataCacheFactory extends AbstractDatabaseMetaDataCacheFactory
{
	private static final long serialVersionUID = -5076802170474579133L;

	/**
	 * {@inheritDoc}
	 * @see net.sf.hajdbc.cache.AbstractDatabaseMetaDataCacheFactory#createCache(net.sf.hajdbc.DatabaseCluster, net.sf.hajdbc.cache.DatabaseMetaDataSupportFactory)
	 */
	@Override
	protected <Z, D extends Database<Z>> DatabaseMetaDataCache<Z, D> createCache(DatabaseCluster<Z, D> cluster, DatabaseMetaDataSupportFactory factory)
	{
		return new SharedLazyDatabaseMetaDataCache<Z, D>(cluster, factory);
	}
}
